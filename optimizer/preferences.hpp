#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

template<typename K>
struct LinearInterpolator {
  std::vector<std::pair<K, double>> points;

  LinearInterpolator(std::initializer_list<std::pair<K, double>> list) : points(list) {
    if (list.size() < 2) {
      throw std::invalid_argument(
          "Not enough points for linear interpolation (need at least 2, got " + std::to_string(list.size()) + ")");
    }
  }

  inline double operator()(K val) const {
    using namespace std;
    // will give either the exact point or immediately above it
    auto lb = lower_bound(begin(points),
                          end(points),
                          make_pair(val, double{}),
                          [](auto const &a, auto const &b) { return a.first < b.first; });
    if (lb == end(points)) {
      return points.back().second;
    } else if (lb == begin(points)) {
      return points.front().second;
    }
    if (not(val == lb->first)) --lb;
    auto next = lb;
    ++next;
    return static_cast<double>(val - lb->first) / (next->first - lb->first) * (next->second - lb->second) + lb->second;
  }
};

class Preference {
  virtual double operator()(Schedule const &) const = 0;
};

namespace pref {

// No, I want to do this at compile time, not runtime.
// Brace yourselves
#define DEFINE_AVG_DAILY_METRIC_PREF(name, preferenceXtype, extract) struct name : public Preference { LinearInterpolator<preferenceXtype> const timePref; \
\
explicit name(const LinearInterpolator<preferenceXtype> &time_pref) : timePref(time_pref) {}\
\
double operator()(Schedule const &sched) const override {\
double score_sum{0.0};\
std::uint8_t count{0};\
for (std::uint8_t d = 0; d < 5; ++d) {\
if (not sched.BlocksOnDay(d).empty()) {                                                                                                                        \
auto blocks = sched.BlocksOnDay(d);                                                                                                                                                               \
score_sum += timePref(extract);\
++count;\
}\
}\
return count == 0 ? 0.5 : score_sum / count;\
}\
};

DEFINE_AVG_DAILY_METRIC_PREF(CompactDays, double, (blocks.back()->End() - blocks.front()->Start()) / 60.0)
DEFINE_AVG_DAILY_METRIC_PREF(EarliestTime, Time, blocks.front()->Start())
DEFINE_AVG_DAILY_METRIC_PREF(LatestTime, Time, blocks.back()->End())

}