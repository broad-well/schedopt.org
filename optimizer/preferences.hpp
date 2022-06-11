#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "schedule.hpp"

template <typename K> struct LinearInterpolator {
  std::vector<std::pair<K, double>> points;

  LinearInterpolator(std::initializer_list<std::pair<K, double>> list)
      : points(list) {
    if (list.size() < 2) {
      throw std::invalid_argument(
          "Not enough points for linear interpolation (need at least 2, got " +
          std::to_string(list.size()) + ")");
    }
  }

  inline double operator()(K val) const {
    using namespace std;
    // will give either the exact point or immediately above it
    auto lb = lower_bound(
        begin(points), end(points), make_pair(val, double{}),
        [](auto const &a, auto const &b) { return a.first < b.first; });
    if (lb == end(points)) {
      return points.back().second;
    } else if (lb == begin(points)) {
      return points.front().second;
    } else if (val == lb->first) return lb->second;
    auto next = lb;
    --lb;
    return static_cast<double>(val - lb->first) / (next->first - lb->first) *
               (next->second - lb->second) +
           lb->second;
  }
};

struct Preference {
  virtual ~Preference() = default;
  virtual double operator()(Schedule const &) const = 0;
  virtual std::string const& Label() const = 0;
};

struct AbsoluteMetric : public Preference {
  virtual double ScaleToPreference(double min, double max, double current) const = 0;
};

namespace pref {

// No, I want to do this at compile time, not runtime.
// Brace yourselves
#define DEFINE_AVG_DAILY_METRIC_PREF(name, label, preferenceXtype, extract)    \
  struct name : public Preference {                                            \
    LinearInterpolator<preferenceXtype> const timePref;                        \
                                                                               \
    explicit name(const LinearInterpolator<preferenceXtype> &time_pref)        \
        : timePref(time_pref) {}                                               \
                                                                               \
    double operator()(Schedule const &sched) const override {                  \
      double score_sum{0.0};                                                   \
      std::uint8_t count{0};                                                   \
      for (std::uint8_t d = 0; d < 5; ++d) {                                   \
        if (not sched.BlocksOnDay(d).empty()) {                                \
          auto blocks = sched.BlocksOnDay(d);                                  \
          score_sum += timePref(extract);                                      \
          ++count;                                                             \
        }                                                                      \
      }                                                                        \
      return count == 0 ? 0.5 : score_sum / count;                             \
    }                                                                          \
                                                                               \
    std::string const& Label() const override {                                \
      static std::string const kLabel{label};                                  \
      return kLabel;                                                           \
    }                                                                          \
};
DEFINE_AVG_DAILY_METRIC_PREF(CompactDays, "Length of school days", double,
                             (blocks.back()->End() - blocks.front()->Start()) /
                                 60.0)
DEFINE_AVG_DAILY_METRIC_PREF(EarliestTime, "Earliest class", Time, blocks.front()->Start())
DEFINE_AVG_DAILY_METRIC_PREF(LatestTime, "Latest class", Time, blocks.back()->End())

struct PreferredInstructors : public Preference {
  // invariant: sorted for binary search and all lowercase
  std::vector<std::string> preferredIds;

  PreferredInstructors(std::initializer_list<std::string> preferred) {
    using namespace std;
    preferredIds.reserve(preferred.size());
    transform(begin(preferred), end(preferred), back_inserter(preferredIds),
              [](auto const &str) {
                auto lower = PreferredInstructors::to_lowercase(str);
                return lower;
              });
    sort(begin(preferredIds), end(preferredIds));
  }

  double operator()(Schedule const &sched) const override {
    if (preferredIds.empty())
      return 0.0;
    unsigned short match_count{0};
    std::vector<bool> matches(preferredIds.size(), false);
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      for (auto const block : sched.BlocksOnDay(day)) {
        if (block->IsClass()) {
          for (auto const &instr : block->details->section.instructors) {
            auto lower = to_lowercase(instr);
            auto target = std::lower_bound(std::begin(preferredIds),
                                           std::end(preferredIds), lower);
            if (target != std::end(preferredIds) and *target == lower) {
              std::size_t idx = std::distance(std::begin(preferredIds), target);
              if (not matches[idx]) {
                ++match_count;
                matches[idx] = true;
              }
            }
          }
        }
      }
    }
    return match_count / static_cast<double>(preferredIds.size());
  }

  std::string const& Label() const override {
    static std::string const kLabel = "Instructors";
    return kLabel;
  }

private:
  static std::string to_lowercase(std::string const &str) {
    using namespace std;
    string copy;
    copy.reserve(str.size());
    transform(begin(str), end(str), back_inserter(copy),
              [](char c) { return tolower(c); });
    return copy;
  }
};

struct PreferredSections : public Preference {
  std::vector<ClassSection const*> preferred;

  PreferredSections(std::initializer_list<ClassSection const*> list): preferred(list) {}

  double operator()(Schedule const& sch) const override {
    if (preferred.empty()) return 0.0;
    using namespace std;
    vector<bool> found(preferred.size(), false);
    uint32_t count = 0;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      for (auto const block: sch.BlocksOnDay(day)) {
        if (not block->IsClass()) continue;
        auto prefIt = find(begin(preferred), end(preferred), &block->details->section);
        if (prefIt != end(preferred)) {
          auto index = distance(begin(preferred), prefIt);
          if (not found[index]) {
            ++count;
            found[index] = true;
          }
        }
      }
    }
    return static_cast<double>(count) / preferred.size();
  }

  std::string const& Label() const override {
    static std::string label = "Sections";
    return label;
  }
};

struct TravelDistance : public AbsoluteMetric {
  LatLong residence;

  explicit TravelDistance(LatLong residence): residence(residence) {}
  
  // meters
  double operator()(Schedule const &sched) const override {
    double dist{0};
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      LatLong loc{residence};
      for (auto const block : sched.BlocksOnDay(day)) {
        if (block->IsClass()) {
          auto next_loc{block->details->location};
          if (next_loc.has_value()) {
            dist += MetersBetween(loc, next_loc.value());
            loc = next_loc.value();
          }
        }
      }
      dist += MetersBetween(loc, residence);
    }
    return dist;
  }

  std::string const& Label() const override {
    static std::string const kLabel = "Weekly travel";
    return kLabel;
  }

  double ScaleToPreference(double min, double max, double current) const override {
    double raw{1.0 - (current - min/2) / (max - min/2)};
    if (raw > 1.0) return 1.0;
    if (raw < 0) return 0.0;
    return raw;
  }
};

class LoadDistribution : public Preference {
  std::vector<double> ideal_load;
public:
  explicit LoadDistribution(std::initializer_list<double> loads): ideal_load(loads) {
    if (ideal_load.size() != kNumWeekdays - 2 and ideal_load.size() != kNumWeekdays) {
      throw std::invalid_argument("Invalid number of load scores: " + std::to_string(ideal_load.size()));
    }
  }

  double operator()(Schedule const &sched) const override {
    double weekly_hour_sum{0};
    std::array<double, kNumWeekdays> hours_per_day;
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const& blocks{sched.BlocksOnDay(day)};
      hours_per_day[day] = 0;
      for (auto const block: blocks) {
        hours_per_day[day] += (block->interval.end - block->interval.start) / 60.0;
      }
      if (day < ideal_load.size()) {
        weekly_hour_sum += hours_per_day[day];
      }
    }
    double daily_mean{weekly_hour_sum / ideal_load.size()};
    if (daily_mean < 1e-5) return 0.5;
    double score_sum{0}, score_min{1};
    for (std::size_t i = 0; i < ideal_load.size(); ++i) {
      double score{1.0 / (1 + Pow6((hours_per_day[i] - (1 + ideal_load[i]) * daily_mean) * 2.0 / daily_mean))};
      score_sum += score;
      if (score < score_min) score_min = score;
    }
    return (score_min + score_sum / ideal_load.size()) / 2;
  }

  std::string const& Label() const override {
    static const std::string label = "Workload distribution";
    return label;
  }

private:
  static inline double Pow6(double x) {
    // inspired by fast exponentiation
    // two products of smaller numbers, one product of larger numbers
    double pow3{x * x * x};
    return pow3 * pow3;
  }
};

} // namespace pref