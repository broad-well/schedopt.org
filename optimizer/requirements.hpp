#pragma once

#include "schedule.hpp"
#include "validators.hpp"
#include <algorithm>

// PreRequirements evaluate class sections, not schedules.
// If a requirement is violated by some schedule iff there is a section in that
// schedule that violates the requirement in any schedule, it is optimized to
// become a PreRequirement, which is used to prune the search space of section
// clusters *before* beginning the search.
class PreRequirement {
  virtual bool CheckSection(ClassSection const &sect) const = 0;
};

namespace req {

struct EarliestClass : public PreRequirement {
  Time const limit;

  explicit EarliestClass(Time const t) : limit(t) {}

  bool CheckSection(ClassSection const &sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks),
                  [this](auto const &block) { return !(block.Start() < limit); });
  }
};

struct LatestClass : public PreRequirement {
  Time const limit;

  explicit LatestClass(Time const t) : limit(t) {}

  bool CheckSection(ClassSection const &sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks),
                  [this](auto const &block) { return !(limit < block.End()); });
  }
};

// ReservedBlocks usage: Use ReservedBlocks to pre-prune sections that overlaps
// with the reserved blocks
//
// Insert all reserved blocks into an empty schedule, then proceed with search
struct ReservedBlocks : public PreRequirement {
  std::vector<TimeBlock> const reserved;
  valid::NoTimeConflicts ntc;

  ReservedBlocks(std::initializer_list<TimeBlock> list) : reserved(list) {}

  // O(n^2) technically, but O(1) average in practice; number of blocks per
  // section is almost always < 3
  bool CheckSection(ClassSection const &sect) const override {
    for (const auto &block : sect.blocks) {
      for (const auto &reserve : reserved) {
        if (block.interval.OverlapsWith(reserve.interval) && (block.days & reserve.days) > 0) {
          return false;
        }
      }
    }
    return true;
  }
};

struct ProhibitedInstructors : public PreRequirement {
  // most blocklists have <5 elements
  // average overhead of following tree node pointers probably as great as 4
  // comparisons (cache locality) therefore also to conserve space, vector used
  // instead of set
  std::vector<std::string> blocklist;

  ProhibitedInstructors(std::initializer_list<std::string> const &elems) {
    using namespace std;
    blocklist.reserve(elems.size());
    for (auto const &str : elems) {
      auto &copy = blocklist.emplace_back();
      copy.reserve(str.size());
      transform(begin(str), end(str), back_inserter(copy),
                [](char c) { return tolower(c); });
    }
  }

  bool CheckSection(ClassSection const &sect) const override {
    auto const &inst = sect.instructors;
    return std::find_first_of(std::begin(inst), std::end(inst),
                              std::begin(blocklist),
                              std::end(blocklist)) == std::end(inst);
  }
};
} // namespace req