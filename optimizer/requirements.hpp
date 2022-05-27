#pragma once

#include "schedule.hpp"
#include "validators.hpp"
#include <algorithm>

class Requirement: public Validator {
public:
  virtual bool CheckSection(ClassSection const& sect) const {
    return true;
  }
};

namespace req {

struct EarliestClass : public Requirement {
  Time const limit;

  explicit EarliestClass(Time const t) : limit(t) {}

  bool operator()(Schedule const &sched) const override {
    using namespace std;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const &blocks = sched.BlocksOnDay(day);
      if (!blocks.empty() && blocks.front()->start < limit) {
        return false;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const &sched,
                      ClassSection const &sect) const override {
    return CheckSection(sect);
  }

  bool CheckSection(ClassSection const &sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks),
                  [this](auto const &block) { return !(block.start < limit); });
  }
};

struct LatestClass : public Requirement {
  Time const limit;

  explicit LatestClass(Time const t) : limit(t) {}

  bool operator()(Schedule const &sched) const override {
    using namespace std;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const &blocks = sched.BlocksOnDay(day);
      if (!blocks.empty() && limit < blocks.back()->end) {
        return false;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const &sched,
                      ClassSection const &sect) const override {
    return CheckSection(sect);
  }

  bool CheckSection(ClassSection const &sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks),
                  [this](auto const &block) { return !(limit < block.end); });
  }
};

// ReservedBlocks usage: Use ReservedBlocks to pre-prune sections that overlaps with the reserved blocks
//
// Insert all reserved blocks into an empty schedule, then proceed with search
struct ReservedBlocks : public Requirement {
  std::vector<TimeBlock> const reserved;
  valid::NoTimeConflicts ntc;

  ReservedBlocks(std::initializer_list<TimeBlock> list) : reserved(list) {}

  bool operator()(Schedule const &sched) const override {
    Schedule schedCopy(sched);
    schedCopy.InsertBlocks(reserved);
    return ntc(schedCopy);
  }

  // O(n^2) technically, but O(1) average in practice; number of blocks per section
  // is almost always < 3
  bool CheckSection(ClassSection const& sect) const override {
    for (const auto& block: sect.blocks) {
      for (const auto& reserve: reserved) {
        if (block.OverlapsWith(reserve) && (block.days & reserve.days) > 0) {
          return false;
        }
      }
    }
    return true;
  }
};
}