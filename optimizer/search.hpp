#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "array.hpp"
#include "preferences.hpp"
#include "requirements.hpp"
#include "schedule.hpp"

struct CourseDetails {
  std::vector<std::vector<std::uint16_t>> clusters;
  std::unordered_map<std::uint16_t, ClassSection> sections;
};

struct ScheduleStats {
  double pref_score;
  Array<double> prefs;
  Array<double> metrics;
};

struct ClusterNode {
  std::variant<
      // cluster index (in CourseDetails) -> Node with that cluster applied
      std::map<std::uint32_t, ClusterNode>, ScheduleStats>
      data;
};

struct SearchResults {
  std::vector<std::string> const &course_order;
  ClusterNode root;

  template <typename T> void ForEachSchedule(T exec) {
    std::vector<std::uint32_t> stack;
    ForEachSchedule(exec, root, stack);
  }

private:
  template <typename T>
  void ForEachSchedule(T exec, ClusterNode &node,
                       std::vector<std::uint32_t> &stack) {
    using namespace std;
    // depth: stack.size()
    // next index of course_order to look, # of clusters in partial
    if (stack.size() == course_order.size()) {
      exec(get<ScheduleStats>(node.data), stack);
    } else {
      auto &children = get<map<uint32_t, ClusterNode>>(node.data);
      for (auto &pair : children) {
        stack.push_back(pair.first);
        ForEachSchedule(exec, pair.second, stack);
        stack.pop_back();
      }
    }
  }
};

struct Search {
  // AFTER filtering by prerequirements (which should be done while loading from
  // db)
  std::unordered_map<std::string, CourseDetails> const &courses;
  std::vector<std::unique_ptr<Validator>> reqs;
  // weighted preferences
  std::vector<std::pair<std::unique_ptr<Preference>, double>> prefs;
  std::vector<std::pair<std::unique_ptr<AbsoluteMetric>, double>> metrics;
  // strategy for scaling metrics to preferences:
  // collect min/max during search. in FindAllSchedules, do search and then additional rating. pass min, max, and each schedule's value
  // for that metric to its scaler, if exists and weight above 1e-10.

  Search(decltype(courses) courses) : courses(courses) {}

  // Conducts a complete search (with backtracking) to find all schedule
  // enrollments
  SearchResults FindAllSchedules() {
    metric_max_min.resize(metrics.size(), {std::numeric_limits<double>::min(), std::numeric_limits<double>::max()});
    course_order = CourseOrderByIncreasingClusterCount();
    partials.resize(courses.size());
    SearchResults results{course_order, RunSearch(0u)};
    results.ForEachSchedule([this](ScheduleStats& stats, auto const& stack) {
      for (std::size_t i = 0; i < metrics.size(); ++i) {
        if (metrics[i].second > 1e-10) {
          stats.pref_score += metrics[i].first->ScaleToPreference(metric_max_min[i].second, metric_max_min[i].first, stats.metrics[i]) * metrics[i].second;
        }
      }
    });
    return results;
  }

private:
  std::vector<Schedule> partials;
  std::vector<std::string> course_order;
  // initialized in FindAllSchedules, populated in RunSearch
  std::vector<std::pair<double, double>> metric_max_min;

  valid::NoTimeConflicts valid_conflicts;
  valid::TravelPractical valid_travel;

  std::vector<std::string> CourseOrderByIncreasingClusterCount() const {
    using namespace std;
    vector<string> out;
    out.reserve(courses.size());
    for (auto const &pair : courses) {
      auto ins = lower_bound(begin(out), end(out), pair.first,
                             [this](auto const &a, auto const &b) {
                               return courses.at(a).clusters.size() <
                                   courses.at(b).clusters.size();
                             });
      out.insert(ins, pair.first);
    }
    assert(out.size() == courses.size());
    return out;
  }

  ClusterNode RunSearch(std::size_t depth) {
    // base case
    if (depth == partials.size()) {
      if (depth == 0) {
        return {ScheduleStats{0, {}, {}}};
      } else {
        auto results = EvaluateSchedule(partials.back());
        RecordMetricsMaxMin(results);
        return {results};
      }
    }

    auto const &details{courses.at(course_order.at(depth))};
    std::map<std::uint32_t, ClusterNode> children;
    for (std::size_t i = 0; i < details.clusters.size(); ++i) {
      auto const &cluster = details.clusters[i];
      bool valid = true;
      partials[depth] = depth == 0 ? Schedule{} : partials[depth - 1];

      for (auto section_num : cluster) {
        if (not CheckInsertion(depth, details.sections.at(section_num))) {
          valid = false;
          break;
        }
        partials[depth].AddSection(details.sections.at(section_num));
      }

      // ignore this cluster if req/validator check failed
      if (not valid)
        continue;
      children[static_cast<std::uint32_t>(i)] = RunSearch(depth + 1);
    }
    return {children};
  }

  bool CheckInsertion(std::size_t depth, ClassSection const &sect) const {
    if (!valid_conflicts.CheckInsertion(partials[depth], sect) or
        !valid_travel.CheckInsertion(partials[depth], sect))
      return false;
    for (auto const &req : reqs) {
      if (!req->CheckInsertion(partials[depth], sect))
        return false;
    }
    return true;
  }

  ScheduleStats EvaluateSchedule(Schedule const &sched) const {
    double score{0};
    Array<double> pref_scores(static_cast<std::uint32_t>(prefs.size()));
    for (std::size_t i = 0; i < prefs.size(); ++i) {
      pref_scores[i] = (*prefs[i].first)(sched);
      score += pref_scores[i] * prefs[i].second;
    }
    ScheduleStats stats{score,
                        std::move(pref_scores),
                        Array<double>(static_cast<std::uint32_t>(metrics.size()))};
    for (std::uint32_t i = 0; i < metrics.size(); ++i) {
      stats.metrics[i] = (*metrics[i].first)(sched);
    }
    return stats;
  }

  void RecordMetricsMaxMin(ScheduleStats const& stats) {
    for (std::size_t i = 0; i < metrics.size(); ++i) {
      auto& existing = metric_max_min[i];
      auto candidate = stats.metrics[i];
      if (candidate > existing.first) existing.first = candidate;
      if (candidate < existing.second) existing.second = candidate;
    }
  }
};