#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "schedule.hpp"
#include "preferences.hpp"
#include "test_common_courses.hpp"

TEST_SUITE("linear interpolation of preferences") {
  TEST_CASE("degenerate interpolation with <2 points") {
    CHECK_THROWS_WITH_AS(LinearInterpolator<Time>({}), "Not enough points for linear interpolation (need at least 2, got 0)", std::invalid_argument);
    CHECK_THROWS_WITH_AS((LinearInterpolator<Time>{{{9, 0}, 0.5}}), "Not enough points for linear interpolation (need at least 2, got 1)", std::invalid_argument);
  }

  TEST_CASE("interpolation with 2 points") {
    LinearInterpolator<Time> li{{{8, 0}, 0.2}, {{11, 0}, 1.0}};
    CHECK_EQ(li({11, 0}), doctest::Approx(1.0));
    CHECK_EQ(li({8, 0}), doctest::Approx(0.2));
    CHECK_EQ(li({10, 0}), doctest::Approx(0.2 + 1.6/3));
  }

  TEST_CASE("interpolation out of range uses closest known value") {
    LinearInterpolator<Time> li{{{8, 0}, 0.1}, {{12, 0}, 0.9}};
    CHECK_EQ(li({7, 59}), doctest::Approx(0.1));
    CHECK_EQ(li({15, 0}), doctest::Approx(0.9));
  }

  TEST_CASE("interpolation with 7 points") {
    LinearInterpolator<Time> li{
        {{8, 0}, 0.4},
        {{9, 0}, 0.6},
        {{10, 0}, 0.8},
        {{10, 30}, 1},
        {{11, 0}, 0.8},
        {{11, 30}, 0.5},
        {{12, 0}, 0.2}
    };
    // before the first point
    CHECK_EQ(li({6, 25}), doctest::Approx(0.4));
    // at the first point
    CHECK_EQ(li({8, 0}), doctest::Approx(0.4));
    // between the first and second points
    CHECK_EQ(li({8, 30}), doctest::Approx(0.5));
    // at the second point
    CHECK_EQ(li({9, 0}), doctest::Approx(0.6));
    // between the second and third points
    CHECK_EQ(li({9, 30}), doctest::Approx(0.7));
    // at the third point
    CHECK_EQ(li({10, 0}), doctest::Approx(0.8));
    // between the third and fourth points
    CHECK_EQ(li({10, 15}), doctest::Approx(0.9));
    CHECK_EQ(li({10, 40}), doctest::Approx(1 - 0.2/3.0));
    CHECK_EQ(li({11, 20}), doctest::Approx(0.6));
    CHECK_EQ(li({11, 50}), doctest::Approx(0.3));
    CHECK_EQ(li({12, 0}), doctest::Approx(0.2));
    CHECK_EQ(li({18, 0}), doctest::Approx(0.2));
  }
}

TEST_SUITE("EarliestTime and LatestTime preferences") {
  TEST_CASE("EarliestTime averages all weekdays with classes for metric") {
    pref::EarliestTime et({
                              {{7, 0}, 0.2},
                              {{9, 0}, 0.8},
                              {{10, 0}, 0.7},
                              {{12, 0}, 0.4}
                          });
    Schedule sched;
    sched.AddSection(eecs183_001);
    sched.AddSection(stats250_204);
    sched.AddSection(ala223_001);
    // earliest times: 11am, 8:30am, 11am, 8:30am, none
    // ratings: 0.55, 0.65, 0.55, 0.65
    // average: 0.6

    CHECK_EQ(et(sched), doctest::Approx(0.6));
  }

  TEST_CASE("EarliestTime on empty schedule gives 0.5") {
    pref::EarliestTime et({{{8, 0}, 0.2}, {{19, 0}, 0.5}});
    Schedule empty;

    CHECK_EQ(et(empty), doctest::Approx(0.5));
  }

  TEST_CASE("LatestTime averages all weekdays with classes for metric") {
    LinearInterpolator<Time> li{
        {{8, 0}, 1.0},
        {{11, 0}, 0.9},
        {{14, 0}, 0.8},
        {{16, 0}, 0.5},
        {{17, 0}, 0.2},
        {{19, 0}, 0.05}
    };
    pref::LatestTime et(li);
    Schedule sched;
    sched.AddSection(eecs183_001);
    sched.AddSection(stats250_204);
    sched.AddSection(math116_023);
    // latest times: none, 4pm, 1pm, 10am, 1pm
    // ratings: 0.5, 0.93333, 0.83333, 0.83333
    // average: 0.775

    CHECK_EQ(et(sched), doctest::Approx(0.775));
  }

  TEST_CASE("LatestTime on empty schedule gives 0.5") {
    pref::LatestTime lt({{{8, 0}, 0.2}, {{19, 0}, 0.5}});
    Schedule empty;

    CHECK_EQ(lt(empty), doctest::Approx(0.5));
  }
}

TEST_SUITE("CompactDays") {
  TEST_CASE("CompactDays averages all weekdays with classes for metric") {
    pref::CompactDays cd{{{3, 0.9}, {5, 0.6}, {7, 0.1}}};
    Schedule sched;
    sched.AddSection(eecs183_001); // 8:30-10:00 01010
    sched.AddSection(stats250_204); // 13:00-16:00 01000
    sched.AddSection(math116_023); // 11:30-13:00 01101
    // Mon: none, Tue: 8:30 to 16, Wed: 11:30 to 13:00, Thu: 8:30 to 10:00, Fri: 11:30 to 13:00
    // durations: 7.5, 1.5, 1.5, 1.5
    // ratings: 0.1, 0.9, 0.9, 0.9
    // average: 0.7

    CHECK_EQ(cd(sched), doctest::Approx(0.7));
  }

  TEST_CASE("CompactDays on empty schedule gives 0.5") {
    pref::CompactDays cd{{{3, 0.9}, {5, 0.6}, {7, 0.1}}};
    Schedule empty;

    CHECK_EQ(cd(empty), doctest::Approx(0.5));
  }
}

TEST_SUITE("PreferredInstructors") {
  // proportion of preferred instructors found in schedule is the score

  TEST_CASE("Empty PreferredInstructors gives 0") {
    pref::PreferredInstructors pi{};
    Schedule empty;
    Schedule loaded;
    loaded.AddSection(stats250_204);
    loaded.AddSection(math116_023);

    CHECK_EQ(pi(empty), doctest::Approx(0));
    CHECK_EQ(pi(loaded), doctest::Approx(0));
  }

  TEST_CASE("Loaded PreferredInstructors on empty schedule gives 0") {
    pref::PreferredInstructors pi{"Rabbit", "God"};
    Schedule empty;

    CHECK_EQ(pi(empty), doctest::Approx(0));
  }

  TEST_CASE("PreferredInstructors on schedule with no preferred instructors gives 0") {
    pref::PreferredInstructors pi{"jjuett", "jbbeau"};
    Schedule loaded;
    loaded.AddSection(ala223_001);
    loaded.AddSection(eecs183_001);

    CHECK_EQ(pi(loaded), doctest::Approx(0));
  }

  TEST_CASE("PreferredInstructors compares IDs case-insensitively") {
    pref::PreferredInstructors pi{"EbFrEtZ"};
    Schedule ala;
    ala.AddSection(ala223_001);
    CHECK_EQ(pi(ala), doctest::Approx(1));
  }

  TEST_CASE("PreferredInstructors ignores non-course time blocks") {
    pref::PreferredInstructors pi{"BENTORRA", "AlRoMeRo", "ccccc"};
    Schedule sched;
    sched.AddSection(eecs183_001);
    sched.AddSection(stats250_204); // encountered multiple times
    sched.AddSection(ala223_001); // searches for ebfretz, past the last preferred
    sched.InsertBlocks(std::vector<TimeBlock>{{{16, 0}, {19, 0}, 0b0010100}});

    CHECK_EQ(pi(sched), doctest::Approx(2.0 / 3.0));
  }
}

TEST_SUITE("TravelDistance") {

}