#pragma once
#include <algorithm>
#include <cmath>
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

class Preference {
  virtual double operator()(Schedule const &) const = 0;
};

namespace pref {

// No, I want to do this at compile time, not runtime.
// Brace yourselves
#define DEFINE_AVG_DAILY_METRIC_PREF(name, preferenceXtype, extract)           \
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
  };

DEFINE_AVG_DAILY_METRIC_PREF(CompactDays, double,
                             (blocks.back()->End() - blocks.front()->Start()) /
                                 60.0)
DEFINE_AVG_DAILY_METRIC_PREF(EarliestTime, Time, blocks.front()->Start())
DEFINE_AVG_DAILY_METRIC_PREF(LatestTime, Time, blocks.back()->End())

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

struct TravelDistance {
  LatLong residence;

  // meters
  double operator()(Schedule const &sched) const {
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
};

} // namespace pref