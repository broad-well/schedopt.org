#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "preferences.hpp"
#include "requirements.hpp"
#include "schedule.hpp"

struct CourseDetails {
  std::vector<std::vector<std::uint16_t>> clusters;
  std::unordered_map<std::uint16_t, ClassSection> sections;
};

// Lightweight array<double> with size immutable after creation but dynamic (can
// be determined at runtime)
template<class T>
class Array {
  T *data;
  std::uint32_t size;

public:
  // not zeroed!
  explicit Array(std::uint32_t size) : data(new T[size]), size(size) {}

  Array(std::initializer_list<T> list)
      : data(new T[list.size()]), size(list.size()) {
    std::uint32_t index = 0;
    for (T const& d : list) {
      data[index++] = d;
    }
  }

  Array(Array<T> const &other)
      : data(new T[other.size]), size(other.size) {
    for (std::uint32_t i = 0; i < other.size; ++i) {
      data[i] = other[i];
    }
  }

  Array &operator=(Array<T> const &other) {
    if (this == &other)
      return *this;
    Array tmp(other);
    std::swap(tmp.data, data);
    std::swap(tmp.size, size);
    return *this;
  }

  Array(Array &&other) : data(other.data), size(other.size) {
    other.data = nullptr;
    other.size = 0;
  }

  std::uint32_t Size() const { return size; }

  T operator[](std::uint32_t i) const {
#ifndef NDEBUG
    assert(i < size);
#endif
    return data[i];
  }

  T &operator[](std::uint32_t i) {
#ifndef NDEBUG
    assert(i < size);
#endif
    return data[i];
  }

  ~Array() { delete[] data; }
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

  template <typename T> void ForEachSchedule(T exec) const {
    std::vector<std::uint32_t> stack;
    ForEachSchedule(exec, root, stack);
  }

private:
  template <typename T>
  void ForEachSchedule(T exec, ClusterNode const &node,
                       std::vector<std::uint32_t> &stack) const {
    using namespace std;
    // depth: stack.size()
    // next index of course_order to look, # of clusters in partial
    if (stack.size() == course_order.size()) {
      exec(get<ScheduleStats>(node.data), stack);
    } else {
      auto const &children = get<map<uint32_t, ClusterNode>>(node.data);
      for (auto const &pair : children) {
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
  std::vector<std::unique_ptr<AbsoluteMetric>> metrics;

  Search(decltype(courses) courses) : courses(courses) {}

  // Conducts a complete search (with backtracking) to find all schedule
  // enrollments
  SearchResults FindAllSchedules() {
    course_order = CourseOrderByIncreasingClusterCount();
    partials.resize(courses.size());
    return {course_order, RunSearch(0u)};
  }

private:
  std::vector<Schedule> partials;
  std::vector<std::string> course_order;

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
      return depth == 0 ? ClusterNode{ScheduleStats{0, {}, {}}}
                        : ClusterNode{EvaluateSchedule(partials.back())};
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
      stats.metrics[i] = (*metrics[i])(sched);
    }
    return stats;
  }
};