#pragma once

#include "schedule.hpp"

class Validator {
 public:
  // True iff valid
  virtual bool operator()(Schedule const&) const = 0;
  // True: Checked, inserted
  // False: Check failed, resulting schedule is partial; discard it
  virtual bool CheckedInsert(Schedule& sched, ClassSection const& sect) const {
    if (CheckInsertion(sched, sect)) {
      sched.AddSection(sect);
      return true;
    }
    return false;
  }
  virtual bool CheckInsertion(Schedule const& sched, ClassSection const& sect) const {
    Schedule newSched(sched);
    newSched.AddSection(sect);
    return (*this)(newSched);
  }
};

namespace valid {

class NoTimeConflicts: public Validator {
 public:
  bool operator()(Schedule const& sched) const override {
    for (unsigned int i = 0; i < kNumWeekdays; ++i) {
      auto const& blocks {sched.BlocksOnDay(i)};
      auto it1 = std::begin(blocks), it2 = it1;
      if (it1 != std::end(blocks)) {
        ++it2;
        for (; it2 != std::end(blocks); ++it1, ++it2) {
          if ((*it1)->OverlapsWith(**it2)) {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const& sched, ClassSection const& sect) const override {
    using namespace std;
    for (auto const& blk: sect.blocks) {
      auto days {blk.days};
      for (int d = kNumWeekdays - 1; d >= 0; --d, days >>= 1) {
        if (days & 1) {
          auto const& blocksOnDay = sched.BlocksOnDay(static_cast<std::uint8_t>(d));
          auto insPos = lower_bound(begin(blocksOnDay), end(blocksOnDay), &blk, [](auto const a, auto const b) {
            return a->start < b->start;
          });
          if (insPos != begin(blocksOnDay)) {
            auto prevBlock = insPos;
            --prevBlock;
            if ((*prevBlock)->OverlapsWith(blk)) return false;
          }
          if (insPos != end(blocksOnDay)) {
            if ((*insPos)->OverlapsWith(blk)) return false;
          }
        }
      }
    }
    return true;
  }
};

class TravelPractical: public Validator {
 public:
  bool operator()(Schedule const& sched) const override {
    using namespace std;
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const &blocks = sched.BlocksOnDay(day);
      auto it1 = begin(blocks), it2 = it1;
      if (it2 != end(blocks)) ++it2;
      while (it2 != end(blocks)) {
        auto const* block1 = dynamic_cast<ClassBlock const*>(*it1),
                   *block2 = dynamic_cast<ClassBlock const*>(*it2);
        if (block1 != nullptr && block2 != nullptr &&
            !travel_practical_from_to(*block1, *block2)) return false;
        ++it1;
        ++it2;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const& sched, ClassSection const& section) const override {
    using namespace std;
    for (auto const& blk: section.blocks) {
      auto days {blk.days};
      for (int d = kNumWeekdays - 1; d >= 0; --d, days >>= 1) {
        if (days & 1) {
          auto const& blocksOnDay = sched.BlocksOnDay(static_cast<std::uint8_t>(d));
          auto insPos = lower_bound(begin(blocksOnDay), end(blocksOnDay), &blk, [](auto const a, auto const b) {
            return a->start < b->start;
          });
          if (insPos != begin(blocksOnDay)) {
            auto prevBlock = insPos;
            --prevBlock;
            auto *prevClass = dynamic_cast<ClassBlock const*>(*prevBlock);
            if (prevClass != nullptr && !travel_practical_from_to(*prevClass, blk)) return false;
          }
          if (insPos != end(blocksOnDay)) {
            auto *nextClass = dynamic_cast<ClassBlock const*>(*insPos);
            if (nextClass != nullptr && !travel_practical_from_to(blk, *nextClass)) return false;
          }
        }
      }
    }
    return true;
  }
 private:
  static inline bool travel_practical_from_to(ClassBlock const& b1, ClassBlock const& b2) {
    double dist = MetersBetween(b1, b2);
    auto interval = b1.start - b2.end;
    return dist <= 1000 || interval > 10;
  }
};
}