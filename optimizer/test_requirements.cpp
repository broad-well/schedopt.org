#include "requirements.hpp"
#include "schedule.hpp"
#include <tuple>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("schedule requirements") {
ClassSection eecs183_001{{{{8, 30}, {10, 0}, 0b0101000, {}, eecs183_001}},
                         {"bentorra"},
                         "LEC",
                         10335,
                         1,
                         4};
ClassSection math116_023{{{{11, 30}, {13, 0}, 0b0100100, {}, math116_023},
                          {{11, 30}, {13, 0}, 0b0010000, {}, math116_023}},
                         {},
                         "LEC",
                         11447,
                         23,
                         4};
ClassSection stats250_204{{{{13, 0}, {16, 0}, 0b0100000, {}, stats250_204}},
                          {"alromero"},
                          "LAB",
                          14113,
                          204,
                          4};

TEST_SUITE("earliest time") {

TEST_CASE("all sections (first class at 8:30) satisfy earliest time "
          "requirement of 8:00") {
  req::EarliestClass ec{{8, 0}};

  CHECK(ec.CheckSection(stats250_204));
  CHECK(ec.CheckSection(math116_023));
  CHECK(ec.CheckSection(eecs183_001));
}

TEST_CASE("all sections (first class at 8:30) satisfy earliest time "
          "requirement of 8:30") {
  req::EarliestClass ec{{8, 30}};

  CHECK(ec.CheckSection(stats250_204));
  CHECK(ec.CheckSection(math116_023));
  CHECK(ec.CheckSection(eecs183_001));
}

TEST_CASE("section with first class at 8:30 violates earliest time "
          "requirement of 9:00") {
  req::EarliestClass ec{{9, 0}};

  CHECK(ec.CheckSection(stats250_204));
  CHECK(ec.CheckSection(math116_023));
  CHECK_FALSE(ec.CheckSection(eecs183_001));
}

TEST_CASE("sections with first class at 11:30 satisfy earliest time "
          "requirement of 10:30") {
  req::EarliestClass ec{{10, 30}};

  CHECK(ec.CheckSection(stats250_204));
  CHECK(ec.CheckSection(math116_023));
}

TEST_CASE("CheckSection rejects sections with time blocks extending before "
          "the limit") {
  req::EarliestClass ec{{12, 0}};

  CHECK(ec.CheckSection(stats250_204));
  CHECK_FALSE(ec.CheckSection(eecs183_001));
  CHECK_FALSE(ec.CheckSection(math116_023));
}
}

TEST_SUITE("latest time") {

TEST_CASE("full schedule with last class at 16:00 satisfies latest time "
          "requirement of 16:30") {
  req::LatestClass lc{{16, 30}};

  CHECK(lc.CheckSection(stats250_204));
  CHECK(lc.CheckSection(math116_023));
  CHECK(lc.CheckSection(eecs183_001));
}

TEST_CASE("full schedule with last class at 16:00 satisfies latest time "
          "requirement of 16:00") {
  req::LatestClass lc{{16, 0}};

  CHECK(lc.CheckSection(stats250_204));
  CHECK(lc.CheckSection(math116_023));
  CHECK(lc.CheckSection(eecs183_001));
}

TEST_CASE("full schedule with last class at 16:00 violates latest time "
          "requirement of 15:30") {
  req::LatestClass lc{{15, 30}};

  CHECK_FALSE(lc.CheckSection(stats250_204));
  CHECK(lc.CheckSection(eecs183_001));
  CHECK(lc.CheckSection(math116_023));
}

TEST_CASE("full schedule with last class at 13:00 satisfies latest time "
          "requirement of 15:00") {
  req::LatestClass lc{{15, 0}};

  CHECK(lc.CheckSection(math116_023));
  CHECK(lc.CheckSection(eecs183_001));
}

TEST_CASE("CheckSection rejects sections with time blocks extending past "
          "the limit") {
  req::LatestClass lc{{15, 0}};
  CHECK(lc.CheckSection(math116_023));
  CHECK(lc.CheckSection(eecs183_001));
  CHECK_FALSE(lc.CheckSection(stats250_204));
}
}

TEST_SUITE("reserved blocks") {
TEST_CASE("all sections satisfy requirement with no blocks reserved") {
  req::ReservedBlocks reserve{};

  CHECK(reserve.CheckSection(stats250_204));
  CHECK(reserve.CheckSection(eecs183_001));
  CHECK(reserve.CheckSection(math116_023));
}

TEST_CASE("section satisfies requirement with blocks of overlapping times "
          "but different days") {
  req::ReservedBlocks reserve{{{9, 30}, {10, 30}, 0b1010101}};

  CHECK(reserve.CheckSection(eecs183_001));
}

TEST_CASE("all sections satisfy requirement with blocks of non-overlapping "
          "times") {
  req::ReservedBlocks reserve{{{16, 30}, {18, 0}, 0b1110010}};

  CHECK(reserve.CheckSection(stats250_204));
  CHECK(reserve.CheckSection(eecs183_001));
  CHECK(reserve.CheckSection(math116_023));
}

TEST_CASE("section violates requirement with blocks of overlapping times") {
  req::ReservedBlocks reserve{{{9, 30}, {10, 30}, 0b0001001}};

  CHECK_FALSE(reserve.CheckSection(eecs183_001));
}

TEST_CASE("section violates requirement where only the last block has "
          "conflicts") {
  req::ReservedBlocks reserve{{{9, 0}, {10, 30}, 0b0010111},
                              {{17, 0}, {18, 0}, 0b0001000},
                              {{11, 0}, {12, 0}, 0b0000100}};
  CHECK(reserve.CheckSection(eecs183_001));
  CHECK_FALSE(reserve.CheckSection(math116_023));
  CHECK(reserve.CheckSection(stats250_204));
}
}

TEST_SUITE("prohibited instructors") {
TEST_CASE("all sections satisfy empty blocklist") {
  req::ProhibitedInstructors pi{};
  CHECK(pi.CheckSection(eecs183_001));
  CHECK(pi.CheckSection(math116_023));
  CHECK(pi.CheckSection(stats250_204));
}

TEST_CASE("section with prohibited instructor violates blocklist, "
          "case-insensitive") {
  req::ProhibitedInstructors noBentorra{"BenTorra"};
  CHECK(noBentorra.CheckSection(math116_023));
  CHECK(noBentorra.CheckSection(stats250_204));
  CHECK_FALSE(noBentorra.CheckSection(eecs183_001));
}

TEST_CASE("all sections with prohibited instructors violate blocklist, "
          "case-insensitive") {
  req::ProhibitedInstructors pi{"BenTorra", "ALROMERO", "somefolk"};
  CHECK(pi.CheckSection(math116_023));
  CHECK_FALSE(pi.CheckSection(stats250_204));
  CHECK_FALSE(pi.CheckSection(eecs183_001));
}
}

TEST_SUITE("meal breaks") {
TEST_CASE("empty schedule satisfies meal break requirement") {
  req::MealBreak mb;
  Schedule empty;

  CHECK(mb(empty));
}

TEST_CASE("empty schedule satisfies requirement with timeframe ending at 00:30") {
  req::MealBreak mb({{0, 0}, {0, 30}}, 30);
  Schedule empty;

  CHECK(mb(empty));
}

TEST_CASE("schedule with one class before meal timeframe satisfies meal "
          "break requirement") {
  req::MealBreak mb;
  Schedule one;
  ClassSection beforeMeal{{{{8, 0}, {10, 30}, 0b1111111, {}, beforeMeal}}};

  CHECK(mb.CheckInsertion(one, beforeMeal));
  one.AddSection(beforeMeal);

  CHECK(mb(one));
}

TEST_CASE("schedule with one class crossing into meal timeframe with "
          "remaining 1hr satisfies requirement") {
  req::MealBreak mb({{10, 0}, {14, 0}}, 45);
  Schedule one;
  ClassSection intoMeal{{{{9, 30}, {13, 0}, 0b0010000, {}, intoMeal}}};

  CHECK(mb.CheckedInsert(one, intoMeal));
  CHECK(mb(one));
}

TEST_CASE("schedule with one class eclipsing meal timeframe violates meal break requirement") {
  req::MealBreak mb({{11, 0}, {12, 30}}, 30);

  // eclipsing: 20 (10 mins remain), 30 (0 mins remain), 50 (20 minutes over)
  std::uint8_t mins[3]{20, 30, 50};
  for (auto min_num : mins) {
    Schedule one;
    ClassSection overMeal{{{{10, 30}, {12, min_num}, 0b0000100, {}, overMeal}}};

    CHECK_FALSE(mb.CheckInsertion(one, overMeal));
    one.AddSection(overMeal);
    CHECK_FALSE(mb(one));
  }
}

TEST_CASE("schedule with one class eclipsing meal timeframe except last 30min satisfies meal break requirement") {
  req::MealBreak mb({{11, 0}, {12, 30}}, 30);
  Schedule one;
  ClassSection overMeal{{{{10, 30}, {12, 0}, 0b0000100, {}, overMeal}}};

  CHECK(mb.CheckInsertion(one, overMeal));
  one.AddSection(overMeal);
  CHECK(mb(one));
}

// each class has 10 states: b-d -> a-b*, a-c, a-d*, a-e, b-c*, b-d*, b-e*, c-d*, c-e, d-e*, d-f, e-f
// a-b b-c, a-b b-d, a-b b-e, a-b c-d, a-b c-e, a-b d-e
// a-c c-d, a-c c-e, a-c d-e, a-d d-e
// b-c c-d, b-c c-e, b-c d-e, b-c e-e
// c-c c-c, c-c c-d, c-c c-e, c-c d-e, c-c e-e
// d-e e-e, e-e e-e

TEST_CASE("schedules with two classes satisfies or violates meal break requirement according to expectations") {
  // meal timeframe is 10 to 13, break time 30 minutes
  // final bool: "tight" (if locations far away, then not achievable)
  std::tuple<Interval, Interval, bool> ok_sections[]{
      {{{8, 0}, {10, 0}}, {{10, 0}, {11, 0}}, false},
      {{{8, 0}, {10, 0}}, {{10, 0}, {12, 30}}, false},
      {{{8, 0}, {10, 0}}, {{10, 30}, {13, 0}}, true},
      {{{8, 0}, {10, 0}}, {{12, 0}, {13, 0}}, false},
      {{{8, 0}, {10, 0}}, {{10, 30}, {12, 50}}, true},
      {{{8, 0}, {10, 0}}, {{10, 10}, {12, 30}}, false},
      {{{8, 0}, {10, 0}}, {{10, 30}, {15, 0}}, true},
      {{{8, 0}, {10, 0}}, {{11, 0}, {15, 0}}, false},
      {{{8, 0}, {10, 0}}, {{13, 0}, {15, 0}}, false},
      {{{8, 0}, {11, 0}}, {{11, 30}, {13, 0}}, true},
      {{{8, 0}, {11, 0}}, {{11, 30}, {14, 0}}, true},
      {{{8, 0}, {11, 0}}, {{12, 0}, {14, 0}}, false},
      {{{9, 0}, {11, 0}}, {{13, 0}, {14, 0}}, false},
      {{{9, 0}, {12, 30}}, {{13, 0}, {14, 0}}, true},
      {{{10, 0}, {11, 0}}, {{11, 30}, {13, 30}}, true},
      {{{10, 0}, {11, 0}}, {{11, 45}, {13, 45}}, false},
      {{{10, 0}, {12, 0}}, {{13, 0}, {14, 0}}, false},
      {{{10, 0}, {12, 30}}, {{13, 0}, {14, 0}}, true},
      {{{10, 0}, {12, 30}}, {{15, 0}, {16, 0}}, false},
      {{{10, 0}, {12, 0}}, {{15, 0}, {16, 0}}, false},
      {{{10, 30}, {11, 30}}, {{11, 40}, {12, 50}}, false},
      {{{10, 10}, {11, 10}}, {{11, 45}, {12, 45}}, true},
      {{{10, 5}, {11, 35}}, {{12, 3}, {12, 30}}, false},
      {{{10, 30}, {11, 30}}, {{12, 0}, {12, 58}}, false},
      {{{10, 45}, {11, 45}}, {{12, 0}, {13, 0}}, false},
      {{{10, 15}, {11, 15}}, {{11, 45}, {13, 0}}, true},
      {{{10, 15}, {11, 15}}, {{12, 0}, {13, 0}}, false},
      {{{10, 45}, {11, 45}}, {{12, 0}, {14, 0}}, false},
      {{{10, 15}, {11, 15}}, {{11, 49}, {14, 0}}, true},
      {{{10, 15}, {11, 15}}, {{12, 0}, {13, 30}}, false},
      {{{10, 30}, {13, 0}}, {{13, 0}, {14, 0}}, false},
      {{{10, 15}, {12, 30}}, {{13, 0}, {14, 30}}, true},
      {{{10, 30}, {12, 30}}, {{13, 0}, {15, 0}}, false},
      {{{10, 45}, {12, 45}}, {{15, 0}, {16, 30}}, false},
      {{{10, 15}, {12, 30}}, {{15, 0}, {16, 0}}, false},
      {{{13, 0}, {14, 0}}, {{14, 0}, {15, 0}}, false},
      {{{13, 30}, {15, 0}}, {{16, 0}, {17, 0}}, false}
  };
  Interval bad_sections[][2]{
      {{{8, 0}, {10, 0}}, {{10, 0}, {12, 31}}},
      {{{8, 0}, {10, 0}}, {{10, 0}, {13, 0}}},
      {{{8, 0}, {10, 0}}, {{10, 0}, {15, 0}}},
      {{{8, 0}, {10, 0}}, {{10, 29}, {13, 0}}},
      {{{8, 0}, {10, 0}}, {{10, 29}, {15, 0}}},
      {{{8, 0}, {11, 0}}, {{11, 29}, {13, 0}}},
      {{{8, 0}, {11, 0}}, {{11, 29}, {13, 30}}},
      {{{9, 0}, {12, 31}}, {{13, 0}, {14, 0}}},
      {{{9, 30}, {13, 0}}, {{13, 0}, {14, 30}}},
      {{{10, 0}, {12, 0}}, {{12, 0}, {13, 0}}},
      {{{10, 0}, {12, 0}}, {{12, 20}, {13, 0}}},
      {{{10, 0}, {11, 30}}, {{11, 30}, {14, 30}}},
      {{{10, 0}, {11, 30}}, {{11, 59}, {14, 30}}},
      {{{10, 0}, {12, 31}}, {{13, 0}, {14, 30}}},
      {{{10, 0}, {13, 0}}, {{13, 0}, {15, 0}}},
      {{{10, 0}, {12, 35}}, {{13, 0}, {15, 0}}},
      {{{10, 0}, {12, 31}}, {{14, 0}, {15, 30}}},
      {{{10, 10}, {11, 40}}, {{11, 50}, {12, 50}}},
      {{{10, 15}, {11, 15}}, {{11, 44}, {12, 31}}},
      {{{10, 20}, {11, 20}}, {{11, 30}, {13, 0}}},
      {{{10, 20}, {11, 30}}, {{11, 30}, {13, 0}}},
      {{{10, 0}, {11, 10}}, {{11, 30}, {13, 0}}},
      {{{10, 20}, {11, 20}}, {{11, 30}, {13, 30}}},
      {{{10, 20}, {11, 30}}, {{11, 30}, {13, 45}}},
      {{{10, 0}, {11, 10}}, {{11, 30}, {15, 0}}},
      {{{10, 0}, {12, 40}}, {{13, 0}, {14, 30}}},
      {{{10, 29}, {12, 31}}, {{13, 0}, {15, 0}}},
      {{{10, 15}, {12, 45}}, {{16, 0}, {17, 30}}},
      {{{10, 29}, {12, 31}}, {{15, 30}, {17, 0}}}
  };
  LatLong bbb{-83.71632909000273, 42.29287090470146};
  LatLong eh{-83.7354954554506, 42.27573293856473};
  LatLong ah{-83.74001368977328, 42.27682834522424};

  req::MealBreak mb({{10, 0}, {13, 0}}, 30);
  for (auto const &sects : ok_sections) {
    // local travel check
    Schedule sched;
    auto const &[sect1, sect2, tight] = sects;
    ClassSection section{
        {{sect1.start, sect1.end, 0b0001000, {eh}, section},
         {sect2.start, sect2.end, 0b0001010, {ah}, section}}
    };
    CHECK(mb.CheckInsertion(sched, section));
    CHECK(mb.CheckedInsert(sched, section));
    CHECK(mb(sched));

    // tight travel check
    Schedule tightSched;
    ClassSection tightSection{
        {{sect1.start, sect1.end, 0b0010010, {bbb}, tightSection},
         {sect2.start, sect2.end, 0b1010000, {eh}, tightSection}}
    };
    REQUIRE_EQ(mb.CheckInsertion(tightSched, tightSection), not std::get<2>(sects));
    tightSched.AddSection(tightSection);
    CHECK_EQ(mb(tightSched), not std::get<2>(sects));

    // multiple section check
    Schedule multSched;
    ClassSection multSect1{
        {{sect1.start, sect1.end, 0b0000100, {ah}, multSect1}}
    };
    ClassSection multSect2{
        {{sect2.start, sect2.end, 0b1000100, {eh}, multSect2}}
    };
    CHECK(mb.CheckedInsert(multSched, multSect1));
    CHECK(mb.CheckInsertion(multSched, multSect2));
    multSched.AddSection(multSect2);
    CHECK(mb(multSched));

    Schedule multTightSched;
    ClassSection multTightSect1{
        {{sect1.start, sect1.end, 0b0001001, {eh}, multTightSect1}}
    };
    ClassSection multTightSect2{
        {{sect2.start, sect2.end, 0b1001000, {bbb}, multTightSect2}}
    };
    CHECK(mb.CheckedInsert(multTightSched, multTightSect1));
    CHECK_EQ(mb.CheckInsertion(multTightSched, multTightSect2), not std::get<2>(sects));
    multTightSched.AddSection(multTightSect2);
    CHECK_EQ(mb(multTightSched), not std::get<2>(sects));
  }
  for (auto sects : bad_sections) {
    Schedule sched;
    ClassSection section{
        {{sects[0].start, sects[0].end, 0b0001000, {}, section}, {sects[1].start, sects[1].end, 0b0001010, {}, section}}
    };
    CHECK_FALSE(mb.CheckInsertion(sched, section));
    CHECK_FALSE(mb.CheckedInsert(sched, section));
    CHECK(mb(sched));
    sched.AddSection(section);
    CHECK_FALSE(mb(sched));
  }
}
}
}