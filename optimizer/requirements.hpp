#pragma once

#include "schedule.hpp"
#include "validators.hpp"
#include <algorithm>

using Requirement = Validator;

namespace req {

struct EarliestClass: public Requirement {
  Time const limit;

  explicit EarliestClass(Time const t): limit(t) {}

  bool operator()(Schedule const& sched) const override {
    using namespace std;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const& blocks = sched.BlocksOnDay(day);
      if (!blocks.empty() && blocks.front()->start < limit) {
        return false;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const& sched, ClassSection const& sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks), [this](auto const& block) {
      return !(block.start < limit);
    });
  }
};

struct LatestClass: public Requirement {
  Time const limit;

  explicit LatestClass(Time const t): limit(t) {}

  bool operator()(Schedule const& sched) const override {
    using namespace std;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const& blocks = sched.BlocksOnDay(day);
      if (!blocks.empty() && limit < blocks.back()->end) {
        return false;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const& sched, ClassSection const& sect) const override {
    using namespace std;
    return all_of(begin(sect.blocks), end(sect.blocks), [this](auto const& block) {
      return !(limit < block.end);
    });
  }
};

struct ReservedBlocks: public Requirement {
  std::vector<TimeBlock> const reserved;

  ReservedBlocks(std::initializer_list<TimeBlock> list): reserved(list) {}

  bool operator()(Schedule const& sched) const override {
    return true;
  }
};
}