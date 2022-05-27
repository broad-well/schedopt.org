#pragma once
#include <utility>
#include <cstdint>
#include <bitset>
#include <string>
#include <vector>
#include <boost/geometry.hpp>
#include <optional>

constexpr int kNumWeekdays = 7;

struct Time {
  std::uint8_t const hour;
  std::uint8_t const minute;

  inline auto operator<(Time const other) const {
    return minutes_since_midnight() < other.minutes_since_midnight();
  }
  inline auto operator==(Time const other) const {
    return minutes_since_midnight() == other.minutes_since_midnight();
  }
  inline signed short operator-(Time const other) const {
    return static_cast<signed short>(minutes_since_midnight()) - static_cast<signed short>(other.minutes_since_midnight());
  }

 private:
  inline unsigned short minutes_since_midnight() const {
    return static_cast<unsigned short>(hour) * 60 + static_cast<unsigned short>(minute);
  }
};

struct Interval {
  Time const start;
  Time const end;

  bool OverlapsWith(Interval const& other) const {
    return other.start < end && start < other.end;
  }
};

struct TimeBlock: public Interval {
  std::optional<boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree>>>
      location;
  // Most significant bit is Monday
  // Example: 0b10000 is Monday only
  std::uint8_t days{};

  bool operator<(TimeBlock const &other) const {
    return start < other.start;
  }
};

double MetersBetween(TimeBlock const &from, TimeBlock const &to) {
  if (!from.location.has_value() || !to.location.has_value()) return 0;
  constexpr double kEarthRadius = 6371.0;
  return 1000 * kEarthRadius
      * boost::geometry::distance(from.location.value(),
                                  to.location.value(),
                                  boost::geometry::strategy::distance::haversine());
}

struct ClassSection {
  std::vector<TimeBlock> blocks;
  std::vector<std::string> instructors;
  char type[4];
  std::uint32_t classNum;
  std::uint16_t sectionNum;
  std::uint8_t credits;
};

class Schedule {
  // source of truth stored by the searcher
  // memo: sorted blocks by day
  std::array<std::vector<TimeBlock const *>, 7> blocksByDay;
  std::uint8_t size = 0;

 public:
  void AddSection(ClassSection const &section) {
    using namespace std;
    ++size;
    InsertBlocks(section.blocks);
  }

  // TODO test
  template<typename Col>
  void InsertBlocks(Col const &blocks) {
    for (const auto &blk : blocks) {
      auto days{blk.days};
      for (int i = kNumWeekdays - 1; i >= 0; --i, days >>= 1) {
        if (days & 1) {
          blocksByDay[i].insert(lower_bound(begin(blocksByDay[i]),
                                            end(blocksByDay[i]),
                                            blk,
                                            [](auto const a, auto const &b) {
                                              return *a < b;
                                            }), &blk);
        }
      }
    }
  }

  std::uint8_t NumSections() const { return size; }

  std::vector<TimeBlock const *> const& BlocksOnDay(std::uint8_t day) const {
    return blocksByDay.at(day);
  }

  std::vector<TimeBlock const *> &BlocksOnDay(std::uint8_t day) {
    return blocksByDay[day];
  }
};

static_assert(sizeof(Schedule) < 193);