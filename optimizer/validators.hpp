#pragma once

#include "schedule.hpp"

class Validator {
 public:
  virtual bool operator()(Schedule const&) const = 0;
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

  }
};

class TravelPractical: public Validator {
 public:
  bool operator()(Schedule const& sched) const override {
    return true;
  }
};
}