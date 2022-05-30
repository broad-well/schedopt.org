#include "schedule.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using I = Interval;

TEST_SUITE("schedule elements") {
TEST_SUITE("block time conflicts") {
TEST_CASE("a-b,c-e") {
  CHECK_FALSE(I{{9, 0}, {10, 0}}.OverlapsWith(I{{10, 30}, {11, 30}}));
  CHECK_FALSE(I{{10, 30}, {11, 30}}.OverlapsWith(I{{9, 0}, {10, 0}}));
}
TEST_CASE("a-c,c-e") {
  CHECK_FALSE(I{{9, 0}, {10, 0}}.OverlapsWith(I{{10, 0}, {11, 0}}));
  CHECK_FALSE(I{{10, 0}, {11, 0}}.OverlapsWith(I{{9, 0}, {10, 0}}));
}
TEST_CASE("a-d,c-e") {
  CHECK(I{{9, 0}, {10, 30}}.OverlapsWith(I{{10, 0}, {11, 30}}));
  CHECK(I{{10, 0}, {11, 30}}.OverlapsWith(I{{9, 0}, {10, 30}}));
}
TEST_CASE("a-e,c-e") {
  CHECK(I{{9, 0}, {11, 0}}.OverlapsWith(I{{10, 0}, {11, 0}}));
  CHECK(I{{10, 0}, {11, 0}}.OverlapsWith(I{{9, 0}, {11, 0}}));
}
TEST_CASE("a-f,c-e") {
  CHECK(I{{9, 0}, {12, 0}}.OverlapsWith(I{{10, 0}, {11, 0}}));
  CHECK(I{{10, 0}, {11, 0}}.OverlapsWith(I{{9, 0}, {12, 0}}));
}
TEST_CASE("c-d,c-e") {
  CHECK(I{{17, 0}, {18, 0}}.OverlapsWith(I{{17, 0}, {18, 30}}));
  CHECK(I{{17, 0}, {18, 30}}.OverlapsWith(I{{17, 0}, {18, 0}}));
}
TEST_CASE("c-e,c-e") {
  CHECK(I{{17, 0}, {18, 30}}.OverlapsWith(I{{17, 0}, {18, 30}}));
}
TEST_CASE("c-f,c-e") {
  CHECK(I{{17, 0}, {18, 30}}.OverlapsWith(I{{17, 0}, {18, 0}}));
  CHECK(I{{17, 0}, {18, 0}}.OverlapsWith(I{{17, 0}, {18, 30}}));
}
TEST_CASE("d-e,c-e") {
  CHECK(I{{17, 30}, {18, 30}}.OverlapsWith(I{{17, 0}, {18, 30}}));
  CHECK(I{{17, 0}, {18, 30}}.OverlapsWith(I{{17, 30}, {18, 30}}));
}
TEST_CASE("d-f,c-e") {
  CHECK(I{{17, 30}, {18, 30}}.OverlapsWith(I{{17, 0}, {18, 0}}));
  CHECK(I{{17, 0}, {18, 0}}.OverlapsWith(I{{17, 30}, {18, 30}}));
}
TEST_CASE("e-f,c-e") {
  CHECK_FALSE(I{{14, 0}, {17, 0}}.OverlapsWith(I{{12, 0}, {14, 0}}));
  CHECK_FALSE(I{{12, 0}, {14, 0}}.OverlapsWith(I{{14, 0}, {17, 0}}));
}
}

TEST_SUITE("distance between blocks") {
TEST_CASE("example across campus") {
  TimeBlock cccb{{{10, 0}, {11, 0}}, {{-83.734924, 42.278301}}, 0b0101000};
  TimeBlock beyster{{{11, 30}, {12, 30}}, {{-83.716257, 42.292781}}, 0b1010000};
  CHECK_LT(abs(MetersBetween(cccb, beyster) - 2220), 5);
}
}

TEST_SUITE("adding sections to schedule copies") {
TEST_CASE("adding section to copy doesn't add it to original") {
  Schedule sched;
  ClassSection section{{{{{10, 0}, {11, 30}}, {}, 0b0101000}}, {"inst"}, "LEC", 10423, 003, 4};
  Schedule sched2(sched);
  sched2.AddSection(section);
  CHECK_EQ(sched2.Size(), 1);
  CHECK_EQ(sched.Size(), 0);
}

TEST_CASE("adding section populates memo array correctly") {
  Schedule sched;
  ClassSection section{
      {
          {{{10, 0}, {11, 30}}, {}, 0b1010000},
          {{{13, 0}, {14, 30}}, {}, 0b0010001},
          {{{12, 0}, {13, 30}}, {}, 0b1001000},
          {{{9, 0}, {10, 30}}, {}, 0b0101000}
      },
      {"jjuett"},
      "LEC", 32410, 2, 4
  };
  sched.AddSection(section);
  using Blocks = std::vector<TimeBlock const *>;

  CHECK_EQ(sched.BlocksOnDay(0), Blocks{&section.blocks[0], &section.blocks[2]});
  CHECK_EQ(sched.BlocksOnDay(1), Blocks{&section.blocks[3]});
  CHECK_EQ(sched.BlocksOnDay(2), Blocks{&section.blocks[0], &section.blocks[1]});
  CHECK_EQ(sched.BlocksOnDay(3), Blocks{&section.blocks[3], &section.blocks[2]});
  CHECK_EQ(sched.BlocksOnDay(4), Blocks{});
  CHECK_EQ(sched.BlocksOnDay(6), Blocks{&section.blocks[1]});
}

TEST_CASE("adding multiple sections populates memo array correctly") {
  Schedule sched;
  ClassSection section0{
      {{{{9, 0}, {10, 30}}, {}, 0b0101000}},
      {"jschatz"},
      "LEC", 35521, 1, 4
  };
  ClassSection section1{
      {
          {{{10, 0}, {11, 30}}, {}, 0b1010000},
          {{{13, 0}, {14, 30}}, {}, 0b0010001}
      },
      {"jjuett"},
      "LEC", 32410, 2, 4
  };
  ClassSection section2{
      {
          {{{12, 0}, {13, 30}}, {}, 0b1001000}
      },
      {"jbbeau"},
      "LEC", 34421, 3, 4
  };
  sched.AddSection(section0);
  sched.AddSection(section1);
  sched.AddSection(section2);
  using Blocks = std::vector<TimeBlock const *>;

  CHECK_EQ(sched.BlocksOnDay(0), Blocks{&section1.blocks[0], &section2.blocks[0]});
  CHECK_EQ(sched.BlocksOnDay(1), Blocks{&section0.blocks[0]});
  CHECK_EQ(sched.BlocksOnDay(2), Blocks{&section1.blocks[0], &section1.blocks[1]});
  CHECK_EQ(sched.BlocksOnDay(3), Blocks{&section0.blocks[0], &section2.blocks[0]});
  CHECK_EQ(sched.BlocksOnDay(4), Blocks{});
  CHECK_EQ(sched.BlocksOnDay(6), Blocks{&section1.blocks[1]});
}
}
}

