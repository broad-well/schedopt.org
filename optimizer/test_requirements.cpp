#include "requirements.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("schedule requirements") {
  ClassSection eecs183_001 {
      {{{{8, 30}, {10, 0}}, {}, 0b0101000}},
      {"bentorra"}, "LEC", 10335, 1, 4
  };
  ClassSection math116_023 {
    {{{{11, 30}, {13, 0}}, {}, 0b0100100},
     {{{11, 30}, {13, 0}}, {}, 0b0010000}},
{}, "LEC", 11447, 23, 4
  };
  ClassSection stats250_204 {
      {{{{13, 0}, {16, 0}}, {}, 0b0100000}},
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

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec.CheckedInsert(sched, eecs183_001));
      CHECK(ec(sched));
    }

    TEST_CASE("full schedule with first class at 8:30 satisfies earliest time requirement of 8:30") {
      Schedule sched;
      req::EarliestClass ec{{8, 30}};

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec.CheckedInsert(sched, eecs183_001));
      CHECK(ec(sched));
    }

    TEST_CASE("full schedule with first class at 8:30 violates earliest time requirement of 9:00") {
      Schedule sched;
      req::EarliestClass ec{{9, 0}};

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK_FALSE(ec.CheckedInsert(sched, eecs183_001));
    }

    TEST_CASE("full schedule with first class at 11:30 satisfies earliest time requirement of 10:30") {
      Schedule sched;
      req::EarliestClass ec{{10, 30}};

      CHECK(ec.CheckedInsert(sched, stats250_204));
      CHECK(ec.CheckedInsert(sched, math116_023));
      CHECK(ec(sched));
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

    CHECK(lc.CheckedInsert(sched, stats250_204));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc(sched));
  }

  TEST_CASE("full schedule with last class at 16:00 satisfies latest time requirement of 16:00") {
    Schedule sched;
    req::LatestClass lc{{16, 0}};

    CHECK(lc.CheckedInsert(sched, stats250_204));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc(sched));
  }

  TEST_CASE("full schedule with last class at 16:00 violates latest time requirement of 15:30") {
    Schedule sched;
    req::LatestClass lc{{15, 30}};

    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK_FALSE(lc.CheckedInsert(sched, stats250_204));
  }

  TEST_CASE("full schedule with last class at 13:00 satisfies latest time requirement of 15:00") {
    Schedule sched;
    req::LatestClass lc{{15, 0}};

    CHECK(lc.CheckedInsert(sched, eecs183_001));
    CHECK(lc.CheckedInsert(sched, math116_023));
    CHECK(lc(sched));
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
          {{{9, 0}, {10, 30}}, {}, 0b0101000},
          {{{16, 0}, {17, 30}}, {}, 0b0101000},
          {{{14, 0}, {18, 0}}, {}, 0b0010101}
      };

      CHECK(reserve(empty));
    }

    TEST_CASE("light schedule satisfies requirement with blocks of overlapping times but different days") {
      Schedule sched;
      req::ReservedBlocks reserve{
          {{{9, 30}, {10, 30}}, {}, 0b1010101}
      };

      CHECK(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK(reserve(sched));
    }

    TEST_CASE("light schedule violates requirement with blocks of overlapping times") {
      Schedule sched;
      req::ReservedBlocks reserve{
          {{{9, 30}, {10, 30}}, {}, 0b0001001}
      };

      CHECK_FALSE(reserve.CheckInsertion(sched, eecs183_001));
      sched.AddSection(eecs183_001);
      CHECK_FALSE(reserve(sched));
      // implementation: should avoid repetition of NoTimeConflicts
    }
  }
}