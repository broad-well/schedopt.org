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
    return all_of(
        begin(sect.blocks), end(sect.blocks),
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
        if (block.interval.OverlapsWith(reserve.interval) &&
            (block.days & reserve.days) > 0) {
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

struct MealBreak : public Validator {
  Interval timeframe;
  std::uint16_t break_minutes;

  MealBreak(Interval const timeframe = {{10, 30}, {14, 30}},
            std::uint16_t break_minutes = 45)
      : timeframe(timeframe), break_minutes(break_minutes) {}

  bool operator()(Schedule const& sched) const override {
    using namespace std;
    for (uint8_t day = 0; day < kNumWeekdays; ++day) {
      Time gap_start {0, 0};
      optional<LatLong> gap_origin;
      bool ok_gap_found = false;
      
      for (auto const block: sched.BlocksOnDay(day)) {
        // default: it's okay to eat during reserved blocks
        if (block->IsClass()) {
          Interval gap {gap_start, block->Start()};
          if (gap.end - gap.start > 0 and gap.OverlapsWith(timeframe)) {
            short mins_overlap = abs(min(timeframe.end, gap.end) - max(timeframe.start, gap.start));
            short contiguous_mins_outside = max(max(short{0}, timeframe.start - gap.start), gap.end - timeframe.end);
            if (gap_origin.has_value() and block->details->location.has_value() and travel_intensive(gap_origin.value(), block->details->location.value())) {
              // avg 15 mins taken to travel intensively (walking or by bus)
              contiguous_mins_outside -= 15;
              if (contiguous_mins_outside < 0) mins_overlap += contiguous_mins_outside;
            }
            if (mins_overlap >= break_minutes) {
              // this day has an eligible break
              ok_gap_found = true;
              break;
            }
          }
          gap_start = block->End();
          gap_origin = block->details->location;
        }
      }
      // BUG timeframe ends really early (00:30)
      if (not ok_gap_found and timeframe.end - gap_start < static_cast<short>(break_minutes)) {
        return false;
      }
    }
    return true;
  }

  // Still need to optimize CheckInsertion
};

} // namespace req