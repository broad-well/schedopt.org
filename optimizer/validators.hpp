#pragma once

#include "schedule.hpp"


template<typename T>
inline bool travel_intensive(T const &b1, T const& b2) {
  return MetersBetween(b1, b2) > 1000;
}

inline bool travel_practical_from_to(TimeBlock const &b1, TimeBlock const &b2) {
  auto interval = b1.Start() - b2.End();
  return not travel_intensive(b1, b2) or interval > 10;
}

class Validator {
public:
  // True iff valid
  virtual bool operator()(Schedule const &) const = 0;
  // True: Checked, inserted
  // False: Check failed, resulting schedule is partial; discard it
  virtual bool CheckedInsert(Schedule &sched, ClassSection const &sect) const {
    if (CheckInsertion(sched, sect)) {
      sched.AddSection(sect);
      return true;
    }
    return false;
  }
  virtual bool CheckInsertion(Schedule const &sched,
                              ClassSection const &sect) const {
    Schedule newSched(sched);
    newSched.AddSection(sect);
    return (*this)(newSched);
  }
};

namespace valid {

class NoTimeConflicts : public Validator {
public:
  bool operator()(Schedule const &sched) const override {
    for (unsigned int i = 0; i < kNumWeekdays; ++i) {
      auto const &blocks{sched.BlocksOnDay(i)};
      auto it1 = std::begin(blocks), it2 = it1;
      if (it1 != std::end(blocks)) {
        ++it2;
        for (; it2 != std::end(blocks); ++it1, ++it2) {
          if ((*it1)->interval.OverlapsWith((*it2)->interval)) {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const &sched,
                      ClassSection const &sect) const override {
    using namespace std;
    for (auto const &blk : sect.blocks) {
      auto days{blk.days};
      for (int d = kNumWeekdays - 1; d >= 0; --d, days >>= 1) {
        if (days & 1) {
          auto const &blocksOnDay =
              sched.BlocksOnDay(static_cast<std::uint8_t>(d));
          auto insPos = lower_bound(begin(blocksOnDay), end(blocksOnDay), &blk,
                                    [](auto const a, auto const b) {
                                      return a->Start() < b->Start();
                                    });
          if (insPos != begin(blocksOnDay)) {
            auto prevBlock = insPos;
            --prevBlock;
            if ((*prevBlock)->interval.OverlapsWith(blk.interval))
              return false;
          }
          if (insPos != end(blocksOnDay)) {
            if ((*insPos)->interval.OverlapsWith(blk.interval))
              return false;
          }
        }
      }
    }
    return true;
  }
};


class TravelPractical : public Validator {
public:
  bool operator()(Schedule const &sched) const override {
    using namespace std;
    for (std::uint8_t day = 0; day < kNumWeekdays; ++day) {
      auto const &blocks = sched.BlocksOnDay(day);
      auto it1 = begin(blocks), it2 = it1;
      if (it2 != end(blocks))
        ++it2;
      while (it2 != end(blocks)) {
        if (!travel_practical_from_to(**it1, **it2))
          return false;
        ++it1;
        ++it2;
      }
    }
    return true;
  }

  bool CheckInsertion(Schedule const &sched,
                      ClassSection const &section) const override {
    using namespace std;
    for (auto const &blk : section.blocks) {
      auto days{blk.days};
      for (int d = kNumWeekdays - 1; d >= 0; --d, days >>= 1) {
        if (days & 1) {
          auto const &blocksOnDay =
              sched.BlocksOnDay(static_cast<std::uint8_t>(d));
          auto insPos = lower_bound(begin(blocksOnDay), end(blocksOnDay), &blk,
                                    [](auto const a, auto const b) {
                                      return a->Start() < b->Start();
                                    });
          if (insPos != begin(blocksOnDay)) {
            auto prevBlock = insPos;
            --prevBlock;
            if (!travel_practical_from_to(**prevBlock, blk))
              return false;
          }
          if (insPos != end(blocksOnDay)) {
            if (!travel_practical_from_to(blk, **insPos))
              return false;
          }
        }
      }
    }
    return true;
  }
};
} // namespace valid