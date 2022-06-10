#include <cstddef>
#include <initializer_list>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "preferences.hpp"
#include "schedule.hpp"
#include "test_common_courses.hpp"

TEST_SUITE("linear interpolation of preferences") {
  TEST_CASE("degenerate interpolation with <2 points") {
    CHECK_THROWS_WITH_AS(
        LinearInterpolator<Time>({}),
        "Not enough points for linear interpolation (need at least 2, got 0)",
        std::invalid_argument);
    CHECK_THROWS_WITH_AS(
        (LinearInterpolator<Time>{{{9, 0}, 0.5}}),
        "Not enough points for linear interpolation (need at least 2, got 1)",
        std::invalid_argument);
  }

  TEST_CASE("interpolation with 2 points") {
    LinearInterpolator<Time> li{{{8, 0}, 0.2}, {{11, 0}, 1.0}};
    CHECK_EQ(li({11, 0}), doctest::Approx(1.0));
    CHECK_EQ(li({8, 0}), doctest::Approx(0.2));
    CHECK_EQ(li({10, 0}), doctest::Approx(0.2 + 1.6 / 3));
  }

  TEST_CASE("interpolation out of range uses closest known value") {
    LinearInterpolator<Time> li{{{8, 0}, 0.1}, {{12, 0}, 0.9}};
    CHECK_EQ(li({7, 59}), doctest::Approx(0.1));
    CHECK_EQ(li({15, 0}), doctest::Approx(0.9));
  }

  TEST_CASE("interpolation with 7 points") {
    LinearInterpolator<Time> li{{{8, 0}, 0.4}, {{9, 0}, 0.6},  {{10, 0}, 0.8},
                                {{10, 30}, 1}, {{11, 0}, 0.8}, {{11, 30}, 0.5},
                                {{12, 0}, 0.2}};
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
    CHECK_EQ(li({10, 40}), doctest::Approx(1 - 0.2 / 3.0));
    CHECK_EQ(li({11, 20}), doctest::Approx(0.6));
    CHECK_EQ(li({11, 50}), doctest::Approx(0.3));
    CHECK_EQ(li({12, 0}), doctest::Approx(0.2));
    CHECK_EQ(li({18, 0}), doctest::Approx(0.2));
  }
}

TEST_SUITE("EarliestTime and LatestTime preferences") {
  TEST_CASE("EarliestTime averages all weekdays with classes for metric") {
    pref::EarliestTime et(
        {{{7, 0}, 0.2}, {{9, 0}, 0.8}, {{10, 0}, 0.7}, {{12, 0}, 0.4}});
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
    LinearInterpolator<Time> li{{{8, 0}, 1.0},  {{11, 0}, 0.9},
                                {{14, 0}, 0.8}, {{16, 0}, 0.5},
                                {{17, 0}, 0.2}, {{19, 0}, 0.05}};
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
    sched.AddSection(eecs183_001);  // 8:30-10:00 01010
    sched.AddSection(stats250_204); // 13:00-16:00 01000
    sched.AddSection(math116_023);  // 11:30-13:00 01101
    // Mon: none, Tue: 8:30 to 16, Wed: 11:30 to 13:00, Thu: 8:30 to 10:00, Fri:
    // 11:30 to 13:00 durations: 7.5, 1.5, 1.5, 1.5 ratings: 0.1, 0.9, 0.9, 0.9
    // average: 0.7

    CHECK_EQ(cd(sched), doctest::Approx(0.7));
  }

  TEST_CASE("CompactDays on empty schedule gives 0.5") {
    pref::CompactDays cd{{{3, 0.9}, {5, 0.6}, {7, 0.1}}};
    Schedule empty;

    CHECK_EQ(cd(empty), doctest::Approx(0.5));
  }
}

TEST_SUITE("ClassGap") {
  // cases: empty/degenerate, do not consider gaps from/to non-class blocks
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

  TEST_CASE("PreferredInstructors' name is \"Instructors\"") {
    pref::PreferredInstructors pi{};
    CHECK_EQ(pi.Label(), "Instructors");
  }

  TEST_CASE("Loaded PreferredInstructors on empty schedule gives 0") {
    pref::PreferredInstructors pi{"Rabbit", "God"};
    Schedule empty;

    CHECK_EQ(pi(empty), doctest::Approx(0));
  }

  TEST_CASE("PreferredInstructors on schedule with no preferred instructors "
            "gives 0") {
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
    sched.AddSection(
        ala223_001); // searches for ebfretz, past the last preferred
    std::vector<TimeBlock> reserved{{{16, 0}, {19, 0}, 0b0010100}};
    sched.InsertBlocks(reserved);

    CHECK_EQ(pi(sched), doctest::Approx(2.0 / 3.0));
  }
}

TEST_SUITE("TravelDistance") {
  LatLong loc_mojo{-83.73136656773802, 42.27991101688434};
  TEST_CASE("TravelDistance on empty schedule gives 0m") {
    pref::TravelDistance td{loc_mojo};
    Schedule sched;

    CHECK_EQ(td(sched), doctest::Approx(0));
  }

  TEST_CASE("TravelDistance's name is \"Weekly travel\"") {
    pref::TravelDistance td{loc_mojo};
    CHECK_EQ(td.Label(), "Weekly travel");
  }

  TEST_CASE("TravelDistance on light schedule gives straight-line distance") {
    pref::TravelDistance td{loc_mojo};
    Schedule sched;
    sched.AddSection(eecs482_001);
    sched.AddSection(eecs428_001);

    double expected{1780 * 4 + 1930 * 4};
    CHECK_EQ(td(sched), doctest::Approx(expected).epsilon(200));
  }

  TEST_CASE("TravelDistance on heavy schedule gives straight-line distance") {
    pref::TravelDistance td{loc_mojo};
    Schedule sched;
    sched.AddSection(math116_023);
    sched.AddSection(eecs183_001);

    double expected{1660 + 710.72*4 + 632.45*2};
    CHECK_EQ(td(sched), doctest::Approx(expected).epsilon(200));
  }

  TEST_CASE("TravelDistance handles non-class time blocks") {
    pref::TravelDistance td{loc_mojo};
    Schedule sched;
    sched.AddSection(ala223_001);
    sched.AddSection(stats250_204);
    std::vector<TimeBlock> reserved{
      {{17, 0}, {19, 0}, 0b0001000}
    };
    // all component TimeBlocks have to live as long as the schedule
    // because the schedule maintains pointers to them
    sched.InsertBlocks(reserved);

    double expected{710.72 * 6};
    CHECK_EQ(td(sched), doctest::Approx(expected).epsilon(200));
  }
}

TEST_SUITE("LoadDistribution") {
  using namespace std;

  TEST_CASE("LoadDistribution on empty schedule gives 0.5") {
    pref::LoadDistribution ld{0, 0, 0, 0, 0};
    Schedule empty;

    CHECK_EQ(ld(empty), doctest::Approx(0.5));
  }

  TEST_CASE("LoadDistribution all-zeros on perfectly balanced schedule gives 1.0") {
    pref::LoadDistribution ld{0, 0, 0, 0, 0};
    Schedule sched;
    sched.AddSection(eecs482_001);
    sched.AddSection(eecs428_001);
    ClassSection friday_90min{
        {{{10, 30}, {12, 0}, 0b0000100, loc_stamps, friday_90min}}};
    sched.AddSection(friday_90min);

    CHECK_EQ(ld(sched), doctest::Approx(1.0));
  }

  TEST_CASE("LoadDistribution -1,0.5,-1,0.5,0 on 0-3-0-3-2 schedule gives >0.87") {
    pref::LoadDistribution ld{-1, 0.5, -1, 0.5, 0};
    Schedule sched;
    sched.AddSection(eecs428_001);
    sched.AddSection(stats250_200);
    sched.AddSection(eecs183_031);
    CHECK_GT(ld(sched), 0.87);
  }

  TEST_CASE("LoadDistribution 0,0,0.3,-0.3,-0.4 on 4.5-4-5-3-2.5 schedule gives >0.9, but 0.3,0.3,-1,0.3,0 gives <0.5") {
    pref::LoadDistribution ld{0, 0, 0.3, -0.3, -0.4};
    Schedule sched;
    ClassSection eecs183 {{{{13, 0}, {14, 30}, 0b0101000, {}, eecs183}, {{14, 0}, {16, 0}, 0b1000000, {}, eecs183}}};
    ClassSection english125 {{{{10, 0}, {11, 30}, 0b0101000, {}, english125}}};
    ClassSection math115 {{{{10, 0}, {11, 30}, 0b1010100, {}, math115}}};
    ClassSection astro101 {{{{13, 0}, {14, 0}, 0b1010100, {}, astro101}, {{15, 0}, {16, 0}, 0b0100000, {}, astro101}}};
    ClassSection ens344 {{{{19, 0}, {21, 30}, 0b0010000, {}, ens344}}};
    sched.AddSection(eecs183);
    sched.AddSection(english125);
    sched.AddSection(math115);
    sched.AddSection(astro101);
    sched.AddSection(ens344);
    CHECK_GT(ld(sched), 0.9);
    pref::LoadDistribution ld2{0.3, 0.3, -1, 0.3, 0};
    CHECK_LT(ld2(sched), 0.5);
  }

  TEST_CASE("LoadDistribution accepts distributions of size 7 for weekends") {
    pref::LoadDistribution ld{0.25, 0.25, -1, 0.25, 0.25, -1, 0};
    Schedule sched;
    ClassSection class1{{{{9, 0}, {12, 0}, 0b1101100, {}, class1}}};
    ClassSection weekend{{{{10, 0}, {11, 30}, 0b0000001, {}, weekend}}};
    sched.AddSection(class1);
    sched.AddSection(weekend);
    CHECK_GT(ld(sched), 0.9);
  }

  TEST_CASE("LoadDistribution rejects distributions of size other than 5 or 7") {
    CHECK_THROWS_WITH((pref::LoadDistribution{0, 0, 0, 0}), "Invalid number of load scores: 4");
    CHECK_THROWS_WITH((pref::LoadDistribution{0, 0, 0}), "Invalid number of load scores: 3");
    CHECK_THROWS_WITH((pref::LoadDistribution{0, 0, 0, 0, 0, 0}), "Invalid number of load scores: 6");
    CHECK_THROWS_WITH((pref::LoadDistribution{0, 0, 0, 0, 0, 0, 0, 0}), "Invalid number of load scores: 8");
  }
}

TEST_SUITE("PreferredSections") {
  TEST_CASE("Empty set of PreferredSections gives 0") {
    pref::PreferredSections ps{};
    Schedule empty;
    Schedule loaded;
    loaded.AddSection(eecs183_001);

    CHECK_EQ(ps(empty), doctest::Approx(0));
    CHECK_EQ(ps(loaded), doctest::Approx(0));
  }

  TEST_CASE("Having no PreferredSections in schedule causes 0") {
    pref::PreferredSections ps{&stats250_212};
    Schedule loaded;
    loaded.AddSection(stats250_200);
    loaded.AddSection(stats250_204);

    CHECK_EQ(ps(loaded), doctest::Approx(0));
  }

  TEST_CASE("Having all PreferredSections in schedule causes 1") {
    pref::PreferredSections ps{&stats250_212, &eecs183_039};
    Schedule loaded;
    loaded.AddSection(stats250_200);
    loaded.AddSection(stats250_212);
    loaded.AddSection(eecs183_039);

    CHECK_EQ(ps(loaded), doctest::Approx(1));
  }

  TEST_CASE("Having 2 of the 3 PreferredSections causes 0.666...") {
    pref::PreferredSections ps{&eecs428_001, &eecs482_001, &stats250_200};
    Schedule loaded;
    loaded.AddSection(eecs428_001);
    loaded.AddSection(eecs482_001);
    loaded.AddSection(eecs183_001);
    loaded.AddSection(eecs183_039);
    CHECK_EQ(ps(loaded), doctest::Approx(2.0 / 3.0));
  }

  TEST_CASE("PreferredSections handles non-class blocks") {
    pref::PreferredSections ps{&stats250_200, &eecs183_039};
    Schedule loaded;
    loaded.AddSection(stats250_200);
    loaded.InsertBlocks(std::vector<TimeBlock>{
      {{19, 0}, {21, 0}, 0b0000100}
    });
    loaded.AddSection(stats250_212);

    CHECK_EQ(ps(loaded), doctest::Approx(1.0 / 2.0));
  }
}