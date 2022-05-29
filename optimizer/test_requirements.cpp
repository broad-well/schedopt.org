#include "requirements.hpp"
#include <iostream>

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
      std::uint8_t mins[3] {20, 30, 50};
      for (auto min_num: mins) {
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

    // now moving onto 2 classes
    // each class has 10 states: b-d -> a-b*, a-c, a-d*, a-e, b-c*, b-d*, b-e*, c-d*, c-e, d-e*, d-f, e-f
    // a-b b-c, a-b b-d, a-b b-e, a-b c-d, a-b c-e, a-b d-e
    // TODO the following
    // a-c c-d, a-c c-e, a-c d-e
    // a-d d-e

    TEST_CASE("schedules with two classes satisfies or violates meal break requirement according to expectations") {
      // meal timeframe is 10 to 13, break time 30 minutes
      Interval ok_sections[][2] {
        {{{8, 0}, {10, 0}}, {{10, 0}, {11, 0}}},
        {{{8, 0}, {10, 0}}, {{10, 0}, {12, 30}}},
        {{{8, 0}, {10, 0}}, {{10, 30}, {13, 0}}},
        {{{8, 0}, {10, 0}}, {{12, 0}, {13, 0}}},
        {{{8, 0}, {10, 0}}, {{10, 30}, {12, 50}}},
        {{{8, 0}, {10, 0}}, {{10, 10}, {12, 30}}},
        {{{8, 0}, {10, 0}}, {{10, 30}, {15, 0}}},
        {{{8, 0}, {10, 0}}, {{11, 0}, {15, 0}}},
        {{{8, 0}, {10, 0}}, {{13, 0}, {15, 0}}},
      };
      Interval bad_sections[][2] {
        {{{8, 0}, {10, 0}}, {{10, 0}, {12, 31}}},
        {{{8, 0}, {10, 0}}, {{10, 0}, {13, 0}}},
        {{{8, 0}, {10, 0}}, {{10, 0}, {15, 0}}},
        {{{8, 0}, {10, 0}}, {{10, 29}, {13, 0}}},
        {{{8, 0}, {10, 0}}, {{10, 29}, {15, 0}}}
      };
      req::MealBreak mb({{10, 0}, {13, 0}}, 30);
      for (auto sects : ok_sections) {
        Schedule sched;
        ClassSection section{
          {{sects[0].start, sects[0].end, 0b0001000, {}, section}, {sects[1].start, sects[1].end, 0b0001010, {}, section}}
        };
        CHECK(mb.CheckInsertion(sched, section));
        CHECK(mb.CheckedInsert(sched, section));
        CHECK(mb(sched));
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

  // meal break exists iff in the lunch timeframe (10am to 3pm), there exists a
  // contiguous period of time no less than (45 minutes) long with no intensive
  // travel nor ongoing class consider all contiguous periods of free time. edge
  // cases: 0 classes (12 am to 11:59pm gap), 1 class (12 am to start, end to
  // 11:59pm)
}