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
  ClassSection sect{};
  TimeBlock cccb({10, 0}, {11, 0}, 0b0101000, {{-83.734924, 42.278301}}, sect);
  TimeBlock beyster({11, 30}, {12, 30}, 0b1010000, {{-83.716257, 42.292781}}, sect);
  CHECK_LT(abs(MetersBetween(cccb, beyster) - 2220), 5);
}
}

TEST_SUITE("adding sections to schedule copies") {
TEST_CASE("adding section to copy doesn't add it to original") {
  Schedule sched;
  ClassSection section{
    {{{10, 0}, {11, 30}, 0b0101000, {}, section}},
    {"inst"}, "LEC", 10423, 003, 4};
  Schedule sched2(sched);
  sched2.AddSection(section);
  CHECK_EQ(sched2.NumSections(), 1);
  CHECK_EQ(sched.NumSections(), 0);
}

TEST_CASE("adding section populates memo array correctly") {
  Schedule sched;
  ClassSection section{
      {
          {{10, 0}, {11, 30}, 0b1010000, {}, section},
          {{13, 0}, {14, 30}, 0b0010001, {}, section},
          {{12, 0}, {13, 30}, 0b1001000, {}, section},
          {{9, 0}, {10, 30}, 0b0101000, {}, section}
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
      {{{9, 0}, {10, 30}, 0b0101000, {}, section0}},
      {"jschatz"},
      "LEC", 35521, 1, 4
  };
  ClassSection section1{
      {
          {{10, 0}, {11, 30}, 0b1010000, {}, section1},
          {{13, 0}, {14, 30}, 0b0010001, {}, section1}
      },
      {"jjuett"},
      "LEC", 32410, 2, 4
  };
  ClassSection section2{
      {
          {{12, 0}, {13, 30}, 0b1001000, {}, section2}
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

TEST_CASE("adding non-class time blocks populates memo array correctly") {
  std::vector<TimeBlock> blocks{
      {{9, 0}, {10, 30}, 0b0100100},
      {{14, 0}, {17, 0}, 0b1010010},
      {{9, 30}, {11, 0}, 0b0010010},
      {{11, 0}, {12, 0}, 0b1101000},
      {{10, 30}, {12, 0}, 0b0000101},
  };
  Schedule sched;
  sched.InsertBlocks(blocks);
  using Blocks = std::vector<TimeBlock const*>;

  CHECK_EQ(sched.BlocksOnDay(0), Blocks{&blocks[3], &blocks[1]});
  CHECK_EQ(sched.BlocksOnDay(1), Blocks{&blocks[0], &blocks[3]});
  CHECK_EQ(sched.BlocksOnDay(2), Blocks{&blocks[2], &blocks[1]});
  CHECK_EQ(sched.BlocksOnDay(3), Blocks{&blocks[3]});
  CHECK_EQ(sched.BlocksOnDay(4), Blocks{&blocks[0], &blocks[4]});
  CHECK_EQ(sched.BlocksOnDay(5), Blocks{&blocks[2], &blocks[1]});
  CHECK_EQ(sched.BlocksOnDay(6), Blocks{&blocks[4]});
}
}

TEST_SUITE("TimeBlock big three") {
  TEST_CASE("copy constructor copies all members and allocates new details object") {
    ClassSection section;
    TimeBlock original({10, 0}, {11, 30}, 0b1010000, {}, section);
    TimeBlock copy(original);

    CHECK_EQ(copy.Start(), original.Start());
    CHECK_EQ(copy.End(), original.End());
    CHECK_EQ(copy.days, original.days);
    CHECK_EQ(copy.details->location.has_value(), original.details->location.has_value());
    CHECK_EQ(&copy.details->section, &original.details->section);
    CHECK_NE(copy.details, original.details);
  }

  TEST_CASE("copy assignment copies all members and allocates new details object") {
    ClassSection section;
    TimeBlock block2({11, 0}, {12, 30}, 0b0010100, {}, section);
    TimeBlock block3({12, 30}, {14, 0}, 0b0101000, {}, section);
    auto const* details3 = block3.details;

    block3 = block2;
    CHECK_NE(block3.details, details3);
    CHECK_NE(block3.details, block2.details);
    CHECK_EQ(block3.Start(), block2.Start());
    CHECK_EQ(block3.End(), block2.End());
    CHECK_EQ(block3.days, block2.days);
    CHECK_EQ(&block3.details->section, &block2.details->section);
    details3 = block3.details;
    block3 = block3;
    // does not reallocate on self-assignment
    CHECK_EQ(details3, block3.details);
  }
}
}

