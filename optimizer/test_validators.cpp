#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "validators.hpp"

ClassSection eecs{
    {{{{10, 0}, {11, 30}}, {}, 0b0101000}},
    {"mpx"}, "LEC", 23195, 3, 4
};
ClassSection chem{
    {{{{9, 0}, {10, 30}}, {}, 0b1010000}},
    {"castanca"}, "LEC", 18859, 5, 3
};
ClassSection math{
    {{{{12, 0}, {13, 0}}, {}, 0b1010100}},
    {"serinh"}, "LEC", 19974, 90, 4
};
ClassSection digital{
    {{{{16, 0}, {17, 30}}, {}, 0b0101000}},
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
      {{{{10, 0}, {11, 0}}, {}, 0b1000000}},
      {"new"}, "DIS", 28113, 10, 4
  };

  sched.AddSection(eecs);
  sched.AddSection(chem);
  sched.AddSection(section);
  CHECK_FALSE(valid::NoTimeConflicts()(sched));
}

TEST_CASE("ae-bd Friday is a time conflict") {
  Schedule sched;
  ClassSection section{
      {{{{11, 30}, {13, 30}}, {}, 0b0001101}},
      {"old"}, "LEC", 22553, 4, 4
  };
  sched.AddSection(math);
  sched.AddSection(section);
  CHECK_FALSE(valid::NoTimeConflicts()(sched));
}

TEST_CASE("ac-ce Tuesday is not a time conflict") {
  Schedule sched;
  ClassSection section{
      {{{{9, 0}, {10, 0}}, {}, 0b0101010}},
      {"test"}, "LEC", 24160, 3, 4
  };
  sched.AddSection(eecs);
  sched.AddSection(section);
  CHECK(valid::NoTimeConflicts()(sched));
}

TEST_CASE("ce-cf Sunday is a time conflict within a section") {
  Schedule sched;
  ClassSection section{
      {{{{9, 0}, {10, 30}}, {}, 0b0010001},
       {{{9, 0}, {12, 0}}, {}, 0b0100001}},
      {"mpx"}, "LEC", 11141, 5, 3
  };
  sched.AddSection(section);
  CHECK_FALSE(valid::NoTimeConflicts()(sched));
}
}

TEST_SUITE("travel practical") {
  TEST_CASE("empty schedule is practical to travel") {
    Schedule sched;
    CHECK(valid::TravelPractical()(sched));
  }

  TEST_CASE("central-to-central with no gap is practical (Michigan time)") {
    Schedule sched;
    ClassSection digital {
        {{{{14, 30}, {16, 0}}, {{-83.73969633054428, 42.276599866547734}}, 0b0101000}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection physics {
        {{{{16, 0}, {17, 0}}, {{-83.73623127817956, 42.275824608796086}}, 0b0100100}},
        {"physical"}, "LAB", 22411, 19, 4
    };
    sched.AddSection(digital);
    sched.AddSection(physics);
    CHECK(valid::TravelPractical()(sched));
  }

  TEST_CASE("central-to-central with gap is practical (Michigan time)") {
    Schedule sched;
    ClassSection digital {
        {{{{14, 30}, {16, 0}}, {{-83.73969633054428, 42.276599866547734}}, 0b0101000}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection physics {
        {{{{16, 30}, {17, 30}}, {{-83.73623127817956, 42.275824608796086}}, 0b0100100}},
        {"physical"}, "LAB", 22411, 19, 4
    };
    sched.AddSection(digital);
    sched.AddSection(physics);
    CHECK(valid::TravelPractical()(sched));
  }

  TEST_CASE("central-to-north with no gap is not practical") {
    Schedule sched;
    ClassSection digital {
        {{{{14, 30}, {16, 0}}, {{-83.73969633054428, 42.276599866547734}}, 0b0101000}},
        {"digital"}, "LEC", 19952, 1, 3
    };
    ClassSection ioe {
        {{{{16, 0}, {17, 0}}, {{-83.71382276363592, 42.29105755443403}}, 0b0100100}},
        {"ioe"}, "LAB", 22411, 23, 4
    };
    sched.AddSection(digital);
    sched.AddSection(ioe);
    CHECK(valid::TravelPractical()(sched));
  }
  // Remember: incremental checks (Validator.VerifyInsertion)
}
}