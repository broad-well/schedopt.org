#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "validators.hpp"


ClassSection eecs{
    {{{10, 0}, {11, 30}, 0b0101000, {}, eecs}},
    {"mpx"}, "LEC", 23195, 3, 4
};
ClassSection chem{
    {{{9, 0}, {10, 30}, 0b1010000, {}, chem}},
    {"castanca"}, "LEC", 18859, 5, 3
};
ClassSection math{
    {{{12, 0}, {13, 0}, 0b1010100, {}, math}},
    {"serinh"}, "LEC", 19974, 90, 4
};
ClassSection digital{
    {{{16, 0}, {17, 30}, 0b0101000, {}, digital}},
    {"teacher"}, "LEC", 23996, 1, 3
};

TEST_SUITE("schedule validators") {
TEST_SUITE("time conflicts") {
TEST_CASE("empty schedule has no time conflicts") {
  Schedule sched;
  CHECK(valid::NoTimeConflicts()(sched));
}

TEST_CASE("correct, filled schedule has no time conflicts") {
  Schedule sched;

  sched.AddSection(eecs);
  sched.AddSection(chem);
  sched.AddSection(math);
  sched.AddSection(digital);

  CHECK(valid::NoTimeConflicts()(sched));
}

TEST_CASE("ac-bd Monday is a time conflict") {
  Schedule sched;
  ClassSection section{
      {{{10, 0}, {11, 0}, 0b1000000, {}, section}},
      {"new"}, "DIS", 28113, 10, 4
  };

  auto ntc = valid::NoTimeConflicts();
  CHECK(ntc.CheckedInsert(sched, eecs));
  CHECK(ntc.CheckedInsert(sched, chem));
  CHECK_FALSE(ntc.CheckedInsert(sched, section));
}

TEST_CASE("ae-bd Friday is a time conflict") {
  Schedule sched;
  ClassSection section{
      {{{11, 30}, {13, 30}, 0b0001101, {}, section}},
      {"old"}, "LEC", 22553, 4, 4
  };
  auto ntc = valid::NoTimeConflicts();

  CHECK(ntc.CheckedInsert(sched, math));
  CHECK_FALSE(ntc.CheckedInsert(sched, section));
}

TEST_CASE("ac-ce Tuesday is not a time conflict") {
  Schedule sched;
  ClassSection section{
      {{{9, 0}, {10, 0}, 0b0101010, {}, section}},
      {"test"}, "LEC", 24160, 3, 4
  };
  valid::NoTimeConflicts ntc;

  CHECK(ntc.CheckedInsert(sched, eecs));
  CHECK(ntc.CheckedInsert(sched, section));
  CHECK(valid::NoTimeConflicts()(sched));
}

// NOTE: time conflicts within sections are not detected by valid::NoTimeConflicts
//   the registrar may publish clusters with conflicts, but not within sections
}

TEST_SUITE("travel practical") {
  TEST_CASE("empty schedule is practical to travel") {
    Schedule sched;
    CHECK(valid::TravelPractical()(sched));
  }

  TEST_CASE("central-to-central with no gap is practical (Michigan time)") {
    Schedule sched;
    ClassSection digital {
        {{{14, 30}, {16, 0}, 0b0101000, {{-83.73969633054428, 42.276599866547734}}, digital}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection physics {
        {{{16, 0}, {17, 0}, 0b0100100, {{-83.73623127817956, 42.275824608796086}}, physics}},
        {"physical"}, "LAB", 22411, 19, 4
    };
    valid::TravelPractical tp;

    CHECK(tp.CheckedInsert(sched, digital));
    CHECK(tp.CheckedInsert(sched, digital));
    CHECK(tp(sched));
  }

  TEST_CASE("central-to-central with gap is practical (Michigan time)") {
    Schedule sched;
    ClassSection digital {
        {{{14, 30}, {16, 0}, 0b0101000, {{-83.73969633054428, 42.276599866547734}}, digital}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection physics {
        {{{16, 30}, {17, 30}, 0b0100100, {{-83.73623127817956, 42.275824608796086}}, physics}},
        {"physical"}, "LAB", 22411, 19, 4
    };
    valid::TravelPractical tp;

    CHECK(tp.CheckedInsert(sched, digital));
    CHECK(tp.CheckedInsert(sched, physics));
    CHECK(tp(sched));
  }

  TEST_CASE("central-to-north with no gap is not practical") {
    Schedule sched;
    ClassSection digital {
        {{{14, 30}, {16, 0}, 0b0101000, {{-83.73969633054428, 42.276599866547734}}, digital}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection ioe {
        {{{16, 0}, {17, 0}, 0b0100100, {{-83.71382276363592, 42.29105755443403}}, ioe}},
        {"ioe"}, "LAB", 22411, 23, 4
    };
    valid::TravelPractical tp;

    CHECK(tp.CheckedInsert(sched, digital));
    CHECK_FALSE(tp.CheckedInsert(sched, ioe));
  }

TEST_CASE("hill-to-Ross with no gap is not practical") {
  Schedule sched;
  ClassSection lifeScience {
      {{{8, 0}, {9, 30}, 0b0101000, {{-83.73416740767517, 42.27969979870305}}, lifeScience}},
      {"lifesci"}, "LEC", 22444, 1, 4
  };
  ClassSection pubHealth {
      {{{9, 30}, {10, 30}, 0b0000101, {{-83.73018194342629, 42.2802180384397}}, pubHealth}},
      {"digital"}, "LEC", 19952, 1, 3
  };
  ClassSection bus {
      {{{10, 30}, {12, 0}, 0b0100001, {{-83.73788423020557, 42.27300677251735}}, bus}},
      {"business"}, "LEC", 28853, 23, 3
  };
  valid::TravelPractical tp;

  CHECK(tp.CheckedInsert(sched, pubHealth));
  CHECK(tp.CheckedInsert(sched, lifeScience));
  CHECK_FALSE(tp.CheckedInsert(sched, bus));
}

}
}