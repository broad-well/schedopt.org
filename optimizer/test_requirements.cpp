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

    TEST_CASE("all sections (first class at 8:30) satisfy earliest time requirement of 8:00") {
      req::EarliestClass ec{{8, 0}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckSection(math116_023));
      CHECK(ec.CheckSection(eecs183_001));
    }

    TEST_CASE("all sections (first class at 8:30) satisfy earliest time requirement of 8:30") {
      req::EarliestClass ec{{8, 30}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckSection(math116_023));
      CHECK(ec.CheckSection(eecs183_001));
    }

    TEST_CASE("section with first class at 8:30 violates earliest time requirement of 9:00") {
      req::EarliestClass ec{{9, 0}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckSection(math116_023));
      CHECK_FALSE(ec.CheckSection(eecs183_001));
    }

    TEST_CASE("sections with first class at 11:30 satisfy earliest time requirement of 10:30") {
      req::EarliestClass ec{{10, 30}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK(ec.CheckSection(math116_023));
    }

    TEST_CASE("CheckSection rejects sections with time blocks extending before the limit") {
      req::EarliestClass ec{{12, 0}};

      CHECK(ec.CheckSection(stats250_204));
      CHECK_FALSE(ec.CheckSection(eecs183_001));
      CHECK_FALSE(ec.CheckSection(math116_023));
    }
  }

  TEST_SUITE("latest time") {

  TEST_CASE("full schedule with last class at 16:00 satisfies latest time requirement of 16:30") {
    req::LatestClass lc{{16, 30}};

    CHECK(lc.CheckSection(stats250_204));
    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckSection(eecs183_001));
  }

  TEST_CASE("full schedule with last class at 16:00 satisfies latest time requirement of 16:00") {
    req::LatestClass lc{{16, 0}};

    CHECK(lc.CheckSection(stats250_204));
    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckSection(eecs183_001));
  }

  TEST_CASE("full schedule with last class at 16:00 violates latest time requirement of 15:30") {
    req::LatestClass lc{{15, 30}};

    CHECK_FALSE(lc.CheckSection(stats250_204));
    CHECK(lc.CheckSection(eecs183_001));
    CHECK(lc.CheckSection(math116_023));
  }

  TEST_CASE("full schedule with last class at 13:00 satisfies latest time requirement of 15:00") {
    req::LatestClass lc{{15, 0}};

    CHECK(lc.CheckSection(math116_023));
    CHECK(lc.CheckSection(eecs183_001));
  }

  TEST_CASE("CheckSection rejects sections with time blocks extending past the limit") {
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

    TEST_CASE("section satisfies requirement with blocks of overlapping times but different days") {
      req::ReservedBlocks reserve{
          {{9, 30}, {10, 30}, 0b1010101}
      };

      CHECK(reserve.CheckSection(eecs183_001));
    }

    TEST_CASE("all sections satisfy requirement with blocks of non-overlapping times") {
      req::ReservedBlocks reserve{
        {{16, 30}, {18, 0}, 0b1110010}
      };

      CHECK(reserve.CheckSection(stats250_204));
      CHECK(reserve.CheckSection(eecs183_001));
      CHECK(reserve.CheckSection(math116_023));
    }

    TEST_CASE("section violates requirement with blocks of overlapping times") {
      req::ReservedBlocks reserve{
          {{9, 30}, {10, 30}, 0b0001001}
      };

      CHECK_FALSE(reserve.CheckSection(eecs183_001));
    }

    TEST_CASE("section violates requirement where only the last block has conflicts") {
      req::ReservedBlocks reserve{
          {{9, 0}, {10, 30}, 0b0010111},
          {{17, 0}, {18, 0}, 0b0001000},
          {{11, 0}, {12, 0}, 0b0000100}
      };
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

    TEST_CASE("section with prohibited instructor violates blocklist, case-insensitive") {
      req::ProhibitedInstructors noBentorra{"BenTorra"};
      CHECK(noBentorra.CheckSection(math116_023));
      CHECK(noBentorra.CheckSection(stats250_204));
      CHECK_FALSE(noBentorra.CheckSection(eecs183_001));
    }

    TEST_CASE("all sections with prohibited instructors violate blocklist, case-insensitive") {
      req::ProhibitedInstructors pi{"BenTorra", "ALROMERO", "somefolk"};
      CHECK(pi.CheckSection(math116_023));
      CHECK_FALSE(pi.CheckSection(stats250_204));
      CHECK_FALSE(pi.CheckSection(eecs183_001));
    }


  }

//  TEST_SUITE("lunch breaks") {
//    TEST_CASE("")
//  }
}