#include "requirements.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("schedule requirements") {
  ClassSection eecs183_001 {
      {{{8, 30}, {10, 0}, 0b0101000, {}, eecs183_001}},
      {"bentorra"}, "LEC", 10335, 1, 4
  };
  ClassSection math116_023 {
    {{{11, 30}, {13, 0}, 0b0100100, {}, math116_023},
       {{11, 30}, {13, 0}, 0b0010000, {}, math116_023}},
{}, "LEC", 11447, 23, 4
  };
  ClassSection stats250_204 {
      {{{13, 0}, {16, 0}, 0b0100000, {}, stats250_204}},
      {"alromero"}, "LAB", 14113, 204, 4
  };


  TEST_SUITE("earliest time") {
    TEST_CASE("empty schedule satisfies earliest time requirement") {
      Schedule sched;
      req::EarliestClass ec{{10, 0}};

      CHECK(ec(sched));
    }

    TEST_CASE("full schedule with first class at 8:30 satisfies earliest time requirement of 8:00") {
      Schedule sched;
      req::EarliestClass ec{{8, 0}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckSection(math116_023));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec.CheckSection(eecs183_001));
      CHECK(ec.CheckedInsert(sched, eecs183_001));
      CHECK(ec(sched));
    }

    TEST_CASE("full schedule with first class at 8:30 satisfies earliest time requirement of 8:30") {
      Schedule sched;
      req::EarliestClass ec{{8, 30}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckSection(math116_023));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec.CheckSection(eecs183_001));
      CHECK(ec.CheckedInsert(sched, eecs183_001));
      CHECK(ec(sched));
    }

    TEST_CASE("full schedule with first class at 8:30 violates earliest time requirement of 9:00") {
      Schedule sched;
      req::EarliestClass ec{{9, 0}};

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK_FALSE(ec.CheckSection(eecs183_001));
      CHECK_FALSE(ec.CheckedInsert(sched, eecs183_001));
    }

    TEST_CASE("full schedule with first class at 11:30 satisfies earliest time requirement of 10:30") {
      Schedule sched;
      req::EarliestClass ec{{10, 30}};

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec(sched));
    }

    TEST_CASE("CheckSection rejects sections with time blocks extending before the limit") {
      req::EarliestClass ec{{12, 0}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK_FALSE(ec.CheckSection(eecs183_001));
      CHECK_FALSE(ec.CheckSection(math116_023));
    }
  }

  TEST_SUITE("latest time") {
  TEST_CASE("empty schedule satisfies latest time requirement") {
    Schedule sched;
    req::LatestClass lc{{17, 0}};

    CHECK(lc(sched));
  }

  TEST_CASE("full schedule with last class at 16:00 satisfies latest time requirement of 16:30") {
    Schedule sched;
    req::LatestClass lc{{16, 30}};

    CHECK(lc.CheckSection(stats250_204));
    CHECK(lc.CheckedInsert(sched, stats250_204));
    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc.CheckSection(eecs183_001));
    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc(sched));
  }

  TEST_CASE("full schedule with last class at 16:00 satisfies latest time requirement of 16:00") {
    Schedule sched;
    req::LatestClass lc{{16, 0}};

    CHECK(lc.CheckSection(stats250_204));
    CHECK(lc.CheckedInsert(sched, stats250_204));
    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc.CheckSection(eecs183_001));
    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc(sched));
  }

  TEST_CASE("full schedule with last class at 16:00 violates latest time requirement of 15:30") {
    Schedule sched;
    req::LatestClass lc{{15, 30}};

    CHECK_FALSE(lc.CheckSection(stats250_204));
    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK_FALSE(lc.CheckedInsert(sched, stats250_204));
  }

  TEST_CASE("full schedule with last class at 13:00 satisfies latest time requirement of 15:00") {
    Schedule sched;
    req::LatestClass lc{{15, 0}};

    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc.CheckSection(eecs183_001));
    CHECK(lc(sched));
  }

  TEST_CASE("CheckSection rejects sections with time blocks extending past the limit") {
    req::LatestClass lc{{15, 0}};
    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckSection(eecs183_001));
    CHECK_FALSE(lc.CheckSection(stats250_204));
  }
  }

  TEST_SUITE("reserved blocks") {
    TEST_CASE("empty schedule satisfies requirement with no blocks reserved") {
      Schedule empty;
      req::ReservedBlocks reserve{};

      CHECK(reserve(empty));
    }

    TEST_CASE("empty schedule satisfies requirement with some non-overlapping blocks") {
      Schedule empty;
      req::ReservedBlocks reserve{
          {{9, 0}, {10, 30}, 0b0101000},
          {{16, 0}, {17, 30}, 0b0101000},
        {{14, 0}, {18, 0}, 0b0010101}
      };

      CHECK(reserve(empty));
    }

    TEST_CASE("light schedule satisfies requirement with blocks of overlapping times but different days") {
      Schedule sched;
      req::ReservedBlocks reserve{
          {{9, 30}, {10, 30}, 0b1010101}
      };

      CHECK(reserve.CheckSection(eecs183_001));
      CHECK(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK(reserve(sched));
    }

    TEST_CASE("light schedule satisfies requirement with blocks of non-overlapping times") {
      req::ReservedBlocks reserve{
        {{16, 30}, {18, 0}, 0b1110010}
      };
      Schedule sched;

      CHECK(reserve.CheckSection(stats250_204));
      CHECK(reserve.CheckedInsert(sched, stats250_204));
      CHECK(reserve.CheckSection(eecs183_001));
      CHECK(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK(reserve.CheckSection(math116_023));
      sched.AddSection(math116_023);
      CHECK(reserve(sched));
    }

    TEST_CASE("light schedule violates requirement with blocks of overlapping times") {
      Schedule sched;
      req::ReservedBlocks reserve{
          {{9, 30}, {10, 30}, 0b0001001}
      };

      CHECK_FALSE(reserve.CheckSection(eecs183_001));
      CHECK_FALSE(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK_FALSE(reserve(sched));
    }

    TEST_CASE("full schedule violates requirement where only the last block has conflicts") {
      Schedule sched;
      req::ReservedBlocks reserve{
          {{9, 0}, {10, 30}, 0b0010111},
          {{17, 0}, {18, 0}, 0b0001000},
          {{11, 0}, {12, 0}, 0b0000100}
      };
      CHECK(reserve.CheckSection(eecs183_001));
      CHECK(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK_FALSE(reserve.CheckInsertion(sched, math116_023));
      CHECK_FALSE(reserve.CheckedInsert(sched, math116_023));
      CHECK_FALSE(reserve.CheckSection(math116_023));
      CHECK(reserve.CheckSection(stats250_204));
      CHECK(reserve.CheckedInsert(sched, stats250_204));
      CHECK(reserve(sched));
      sched.AddSection(math116_023);
      CHECK_FALSE(reserve(sched));
    }
  }

  TEST_SUITE("prohibited instructors") {
    TEST_CASE("all schedules satisfy empty blocklist") {
      Schedule sEmpty, sLight, sAll;
      sLight.AddSection(eecs183_001);
      sAll.AddSection(math116_023);
      sAll.AddSection(eecs183_001);
      sAll.AddSection(stats250_204);
      req::ProhibitedInstructors pi{};

      CHECK(pi(sEmpty));
      CHECK(pi(sLight));
      CHECK(pi(sAll));
    }

    TEST_CASE("section with prohibited instructor violates blocklist, case-insensitive") {
      req::ProhibitedInstructors noBentorra{"BenTorra"};
      CHECK(noBentorra.CheckSection(math116_023));
      CHECK(noBentorra.CheckSection(stats250_204));
      CHECK_FALSE(noBentorra.CheckSection(eecs183_001));

      Schedule sched;
      CHECK(noBentorra.CheckInsertion(sched, math116_023));
      sched.AddSection(math116_023);
      CHECK(noBentorra.CheckInsertion(sched, stats250_204));
      sched.AddSection(stats250_204);
      CHECK_FALSE(noBentorra.CheckedInsert(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK_FALSE(noBentorra(sched));
    }

    TEST_CASE("all sections with prohibited instructors violate blocklist, case-insensitive") {
      req::ProhibitedInstructors pi{"BenTorra", "ALROMERO", "somefolk"};
      CHECK(pi.CheckSection(math116_023));
      CHECK_FALSE(pi.CheckSection(stats250_204));
      CHECK_FALSE(pi.CheckSection(eecs183_001));

      Schedule sched;
      CHECK(pi.CheckInsertion(sched, math116_023));
      sched.AddSection(math116_023);
      CHECK_FALSE(pi.CheckInsertion(sched, stats250_204));
      CHECK_FALSE(pi.CheckedInsert(sched, eecs183_001));
      sched.AddSection(stats250_204);
      CHECK_FALSE(pi(sched));
    }


  }
}