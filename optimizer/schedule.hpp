#pragma once
#include <bitset>
#include <boost/geometry.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

constexpr int kNumWeekdays = 7;

struct Time {
  std::uint8_t hour;
  std::uint8_t minute;

  inline auto operator<(Time const other) const {
    return minutes_since_midnight() < other.minutes_since_midnight();
  }
  inline auto operator==(Time const other) const {
    return minutes_since_midnight() == other.minutes_since_midnight();
  }
  inline signed short operator-(Time const other) const {
    return static_cast<signed short>(minutes_since_midnight()) -
        static_cast<signed short>(other.minutes_since_midnight());
  }

 private:
  inline unsigned short minutes_since_midnight() const {
    return static_cast<unsigned short>(hour) * 60 +
        static_cast<unsigned short>(minute);
  }
};

struct Interval {
  Time start;
  Time end;

  Interval(Time start, Time end) : start(start), end(end) {}

  bool OverlapsWith(Interval const &other) const {
    return other.start < end && start < other.end;
  }
};

struct ClassSection;

using LatLong = boost::geometry::model::point<
    double, 2, boost::geometry::cs::geographic<boost::geometry::degree>>;

struct ClassBlockDetails {
  std::optional<LatLong> location;
  ClassSection const &section;
};

struct TimeBlock {
  Interval interval;
  // Most significant bit is Monday
  // Example: 0b10000 is Monday only
  std::uint8_t days{};
  ClassBlockDetails const *details = nullptr;

  TimeBlock(Time from, Time to, std::uint8_t days)
      : interval(from, to), days(days) {};

  // In C++20 we would use make_unique with aggregate initialization
  TimeBlock(Time from, Time to, std::uint8_t days,
            decltype(details->location) location, ClassSection const &section)
      : interval(from, to), days(days),
        details(new ClassBlockDetails{location, section}) {}

  TimeBlock(TimeBlock const &o)
      : interval(o.interval), days(o.days),
        details(o.details == nullptr ? nullptr
                                     : new ClassBlockDetails(*o.details)) {}

  TimeBlock &operator=(TimeBlock const &o) {
    if (&o == this) return *this;
    TimeBlock temp(o);
    interval = temp.interval;
    days = temp.days;
    std::swap(details, temp.details);
    return *this;
  }

  ~TimeBlock() { delete details; }

  bool operator<(TimeBlock const &other) const {
    return interval.start < other.interval.start;
  }

  Time Start() const { return interval.start; }
  Time End() const { return interval.end; }

  bool IsClass() const { return details != nullptr; }

  // Each TimeBlock should exist only once in memory
  // Do not copy-construct or copy-assign (except to construct them in tests)
};

inline double MetersBetween(LatLong const &p1, LatLong const &p2) {
  constexpr double kEarthRadius = 6371.0;
  return 1000 * kEarthRadius *
      boost::geometry::distance(p1, p2, boost::geometry::strategy::distance::haversine());
}

inline double MetersBetween(TimeBlock const &from, TimeBlock const &to) {
  if (!from.IsClass() || !from.details->location.has_value() || !to.IsClass() ||
      !to.details->location.has_value())
    return 0;
  return MetersBetween(from.details->location.value(), to.details->location.value());
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
          blocksByDay[i].insert(
              lower_bound(begin(blocksByDay[i]), end(blocksByDay[i]), blk,
                          [](auto const a, auto const &b) { return *a < b; }),
              &blk);
        }
      }
    }
  }

  std::uint8_t NumSections() const { return size; }

  std::vector<TimeBlock const *> const &BlocksOnDay(std::uint8_t day) const {
    return blocksByDay.at(day);
  }

  std::vector<TimeBlock const *> &BlocksOnDay(std::uint8_t day) {
    return blocksByDay[day];
  }
};