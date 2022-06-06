#include "search.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Full schedule discovery") {
  TEST_CASE("No schedules generated from empty sets of clusters") {
    Search search{
      {}, // courses/sections unordered_map<string (course code), {vector<vector<uint16_t>> (clusters), unordered_map<uint16_t (section code), ClassSection>}>
      {}, // prerequirements
      {}, // requirements
      {}, // preferences
      {}, // absolute preferences (output not 0-1, like TravelDistance)
    };

    auto all{search.FindAllSchedules()};
    CHECK(all.empty());
  }

  TEST_CASE("Single schedule generated from a single course with a single section/cluster") {
    ClassSection ners579_001 {
      {{{15, 0}, {16, 30}, 0b0101000, {}, ners579_001}},
      {"jefoster"}, "LEC", 31609, 1, 3
    };
    std::vector<std::uint16_t> cluster{1};
    Search search{
      {{"NERS 579", {{cluster}, {{1, ners579_001}}}}},
      {},
      {},
      {},
      {}
    };

    auto all{search.FindAllSchedules()};
    CHECK_EQ(all.size(), 1u);
    CHECK_EQ(all.front().first.courseClusters, std::vector<std::pair<std::string, std::vector<std::uint16_t>&>>{{"NERS 579", cluster}});
  }
}