#include "search.hpp"
#include "test_common_courses.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Full schedule discovery") {
TEST_CASE("Empty schedule generated from empty sets of clusters") {
  std::unordered_map<std::string, CourseDetails> courses;
  Search search(
      courses // courses/sections unordered_map<string (course code), {vector<vector<uint16_t>> (clusters), unordered_map<uint16_t (section code), ClassSection>}>
  );

  auto all{search.FindAllSchedules()};
  CHECK(all.course_order.empty());
  auto stats{std::get<ScheduleStats>(all.root.data)};
  CHECK_EQ(stats.metrics.Size(), 0);
  CHECK_EQ(stats.pref_score, 0.0);
}

// (these metrics are necessary because not all discovered schedules are stored
//  simultaneously as timetables at some instant, to save memory)
// imagine millions * (sizeof(Schedule) + 24 * 7 + ...)

struct CustomMetricNumSections: public AbsoluteMetric {
  double operator()(Schedule const& s) const override {
    return static_cast<double>(s.NumSections());
  }
  std::string const& Label() const override {
    static std::string const label = "Number of sections";
    return label;
  }
  double ScaleToPreference(double min, double max, double current) const override {
    return 1.0 / current;
  }
};

struct CustomMetricDailyClassCount: public AbsoluteMetric {
  double operator()(Schedule const& s) const override {
    double out{0};
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      out = out * 10 + static_cast<double>(s.BlocksOnDay(day).size());
    }
    return out;
  }
  std::string const& Label() const override {
    static std::string const label = "Number of classes daily";
    return label;
  }
  double ScaleToPreference(double min, double max, double current) const override {
    return current + 2*min + max;
  }
};

TEST_CASE("Single schedule generated from a single course with a single section/cluster") {
  using namespace std;

  ClassSection ners579_001{
      {{{15, 0}, {16, 30}, 0b0101000, {}, ners579_001}},
      {"jefoster"}, "LEC", 31609, 1, 3
  };
  vector<uint16_t> cluster{1};
  unordered_map<string, CourseDetails> courses{{"NERS 579", {{cluster}, {{1, ners579_001}}}}};
  Search search(courses);
  search.metrics.emplace_back(new CustomMetricNumSections(), 0.0);

  auto all{search.FindAllSchedules()};
  CHECK_EQ(all.course_order, vector<string>{"NERS 579"});
  auto const &section = get<map<uint32_t, ClusterNode>>(all.root.data);
  CHECK_EQ(section.size(), 1);
  auto const &sched = get<ScheduleStats>(section.at(0u).data);
  CHECK_EQ(sched.metrics[0], doctest::Approx(1));
}

TEST_CASE("Search discovers all combinations of clusters") {
  using namespace std;

  unordered_map<string, CourseDetails> courses {
      {"STATS 250", {
          {{200, 204}, {200, 212}},
          {
              {200, stats250_200},
              {204, stats250_204},
              {212, stats250_212}
          }
      }},
      {"EECS 183", {
          {{1, 31}, {1, 39}},
          {
              {1, eecs183_001},
              {31, eecs183_031},
              {39, eecs183_039}
          }
      }}
  };
  Search search(courses);
  search.metrics.emplace_back(new CustomMetricNumSections(), 0.0);
  search.metrics.emplace_back(new CustomMetricDailyClassCount(), 0.0);
  auto results{search.FindAllSchedules()};

  CHECK_EQ(results.course_order.size(), 2);

  vector<ScheduleStats> schedules;
  set<vector<uint32_t>> cluster_selections;
  results.ForEachSchedule([&](auto sched, auto const& stack) {
    schedules.emplace_back(sched);
    CHECK_EQ(sched.metrics[0], doctest::Approx(4));
    cluster_selections.emplace(stack);

    // Deeper check of the schedule: replicate the time blocks
    Schedule expected;
    for (size_t i = 0; i < stack.size(); ++i) {
      for (auto sect: courses[results.course_order[i]].clusters[stack[i]]) {
        expected.AddSection(courses[results.course_order[i]].sections[sect]);
      }
    }
    uint32_t dailyCounts = round(sched.metrics[1]);
    for (int8_t day = 6; day >= 0; --day, dailyCounts /= 10) {
      CHECK_EQ(expected.BlocksOnDay(static_cast<uint8_t>(day)).size(), static_cast<size_t>(dailyCounts % 10));
    }
  });
  CHECK_EQ(schedules.size(), 2 * 2);
  CHECK_EQ(cluster_selections, set<vector<uint32_t>>{{0, 0}, {0, 1}, {1, 0}, {1, 1}});
}

TEST_CASE("Search prunes by requirement and scores by preference") {
  using namespace std;

  unordered_map<string, CourseDetails> courses {
      {"STATS 250", {
          {{200, 204}, {200, 212}},
          {
              {200, stats250_200}, // TuTh 10-11:30
              {204, stats250_204}, // violates requirement
              {212, stats250_212}  // We 13-14:30
          }
      }},
      {"EECS 183", {
          {{1, 31}, {1, 39}},
          {
              {1, eecs183_001},  // TuTh 8:30-10
              {31, eecs183_031}, // violates requirement
              {39, eecs183_039}  // Friday 15-17
          }
      }}
  };
  Search search(courses);
  search.reqs.emplace_back(new req::MealBreak({{13, 30}, {15, 30}}));
  LinearInterpolator<Time> interp{
      {{0, 0}, 0},
      {{8, 30}, 0.1},
      {{9, 0}, 0.2},
      {{12, 0}, 0.8},
      {{14, 0}, 1.0},
      {{15, 0}, 1.0},
      {{18, 0}, 0.2}
  };
  LinearInterpolator<Time> latestInterp{
      {{12, 0}, 1},
      {{14, 30}, 0.95},
      {{16, 0}, 0.9},
      {{17, 0}, 0.7},
      {{18, 0}, 0.5},
      {{19, 0}, 0.2},
      {{20, 0}, 0}
  };
  search.prefs.emplace_back(new pref::EarliestTime(interp), 1);
  search.prefs.emplace_back(new pref::LatestTime(latestInterp), 1);
  search.metrics.emplace_back(new CustomMetricNumSections(), 0.0);
  search.metrics.emplace_back(new CustomMetricDailyClassCount(), 0.0);
  auto results{search.FindAllSchedules()};

  CHECK_EQ(results.course_order.size(), 2);

  vector<ScheduleStats> schedules;
  set<vector<uint32_t>> cluster_selections;
  results.ForEachSchedule([&](auto sched, auto const& stack) {
    schedules.emplace_back(sched);
    CHECK_EQ(sched.metrics[0], doctest::Approx(4));
    cluster_selections.emplace(stack);
    CHECK_EQ(round(sched.metrics[1]), 212100);
    CHECK_EQ(sched.prefs[0], doctest::Approx((0.1 + 0.9 + 0.1 + 1.0) / 4.0));
    CHECK_EQ(sched.prefs[1], doctest::Approx((1.0 + 0.95 + 1.0 + 0.7) / 4.0));
    CHECK_EQ(sched.pref_score, doctest::Approx(sched.prefs[0] + sched.prefs[1]));
  });
  CHECK_EQ(schedules.size(), 1);
  CHECK_EQ(cluster_selections, set<vector<uint32_t>>{{1, 1}});
}

TEST_CASE("Search prunes by requirement and scores by preference as well as scaled metrics") {
  using namespace std;

  unordered_map<string, CourseDetails> courses {
      {"STATS 250", {
          {{200, 204}, {200, 212}},
          {
              {200, stats250_200}, // TuTh 10-11:30
              {204, stats250_204}, // violates requirement
              {212, stats250_212}  // We 13-14:30
          }
      }},
      {"EECS 183", {
          {{1, 31}, {1, 39}},
          {
              {1, eecs183_001},  // TuTh 8:30-10
              {31, eecs183_031}, // violates requirement
              {39, eecs183_039}  // Friday 15-17
          }
      }}
  };
  Search search(courses);
  search.reqs.emplace_back(new req::MealBreak({{13, 30}, {15, 30}}));
  LinearInterpolator<Time> interp{
      {{0, 0}, 0},
      {{8, 30}, 0.1},
      {{9, 0}, 0.2},
      {{12, 0}, 0.8},
      {{14, 0}, 1.0},
      {{15, 0}, 1.0},
      {{18, 0}, 0.2}
  };
  LinearInterpolator<Time> latestInterp{
      {{12, 0}, 1},
      {{14, 30}, 0.95},
      {{16, 0}, 0.9},
      {{17, 0}, 0.7},
      {{18, 0}, 0.5},
      {{19, 0}, 0.2},
      {{20, 0}, 0}
  };
  search.prefs.emplace_back(new pref::EarliestTime(interp), 1);
  search.prefs.emplace_back(new pref::LatestTime(latestInterp), 1);
  search.metrics.emplace_back(new CustomMetricNumSections(), 0.3);
  search.metrics.emplace_back(new CustomMetricDailyClassCount(), 0.8);
  auto results{search.FindAllSchedules()};

  CHECK_EQ(results.course_order.size(), 2);

  vector<ScheduleStats> schedules;
  set<vector<uint32_t>> cluster_selections;
  results.ForEachSchedule([&](auto sched, auto const& stack) {
    schedules.emplace_back(sched);
    CHECK_EQ(sched.metrics[0], doctest::Approx(4));
    cluster_selections.emplace(stack);
    CHECK_EQ(round(sched.metrics[1]), 212100);
    CHECK_EQ(sched.prefs[0], doctest::Approx((0.1 + 0.9 + 0.1 + 1.0) / 4.0));
    CHECK_EQ(sched.prefs[1], doctest::Approx((1.0 + 0.95 + 1.0 + 0.7) / 4.0));
    CHECK_EQ(sched.pref_score, doctest::Approx(sched.prefs[0] + sched.prefs[1] + 0.3 / sched.metrics[0] + 0.8 * (sched.metrics[1] + 2 * sched.metrics[1] + sched.metrics[1])));
  });
  CHECK_EQ(schedules.size(), 1);
  CHECK_EQ(cluster_selections, set<vector<uint32_t>>{{1, 1}});
}

TEST_CASE("Search collects min and max of metrics correctly") {
  using namespace std;

  unordered_map<string, CourseDetails> courses {
      {"STATS 250", {
          {{200, 204}, {200, 212}},
          {
              {200, stats250_200}, // TuTh 10-11:30
              {204, stats250_204}, // Tu 13-16
              {212, stats250_212}  // We 13-14:30
          }
      }},
      {"EECS 183", {
          {{1, 31}, {1, 39}},
          {
              {1, eecs183_001},  // TuTh 8:30-10
              {31, eecs183_031}, // Fri 14-16
              {39, eecs183_039}  // Fri 15-17
          }
      }}
  };
  Search search(courses);
  search.metrics.emplace_back(new CustomMetricDailyClassCount, 0.3);

  map<pair<uint32_t, uint32_t>, double> expected_scores {
    // 03021
    {{0, 0}, 302100 + 2*212100 + 302100},
    // 03021
    {{0, 1}, 302100 + 2*212100 + 302100},
    // 02121
    {{1, 0}, 212100 + 2*212100 + 302100},
    // 02121
    {{1, 1}, 212100 + 2*212100 + 302100}
    // min: 02010, max: 03021
    // ScaleToPreference formula for CMDC: current * min + max
  };
  
  auto results = search.FindAllSchedules();
  results.ForEachSchedule([&](auto const& stats, auto const& stack) {
    CHECK_EQ(stats.pref_score, doctest::Approx(0.3 * expected_scores.at({stack[0], stack[1]})));
  });
}

TEST_CASE("Search of a subset of all courses results in schedules containing exactly that subset") {
  using namespace std;

  unordered_map<string, CourseDetails> courses {
      {"STATS 250", {
          {{200, 204}, {200, 212}},
          {
              {200, stats250_200}, // TuTh 10-11:30
              {204, stats250_204}, // Tu 13-16
              {212, stats250_212}  // We 13-14:30
          }
      }},
      {"EECS 183", {
          {{1, 31}, {1, 39}},
          {
              {1, eecs183_001},  // TuTh 8:30-10
              {31, eecs183_031}, // Fri 14-16
              {39, eecs183_039}  // Fri 15-17
          }
      }}
  };
  Search search(courses);
  search.metrics.emplace_back(new CustomMetricDailyClassCount, 0.3);
  
  vector<string> course_combo{"EECS 183"};
  uint16_t schedule_count = 0;
  auto results = search.FindAllSchedules(course_combo);
  results.ForEachSchedule([&](auto const& stats, auto const& stack) {
    CHECK_EQ(stats.metrics[0], 101100);
    ++schedule_count;
  });
  CHECK_EQ(schedule_count, 2);
}
}
