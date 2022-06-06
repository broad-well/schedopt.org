#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>

#include "schedule.hpp"
#include "requirements.hpp"
#include "preferences.hpp"

struct CourseDetails {
    std::vector<std::vector<std::uint16_t>> clusters;
    std::unordered_map<std::uint16_t, ClassSection> sections;
};

// Compact description of what sections to enroll in
struct ScheduleEnrollment {
    std::vector<std::pair<std::string, std::vector<std::uint16_t>&>> courseClusters;
};

struct Search {
    std::unordered_map<std::string, CourseDetails> const& courses;
    std::vector<std::unique_ptr<PreRequirement>> prereqs;
    std::vector<std::unique_ptr<Validator>> reqs;
    std::vector<std::unique_ptr<Preference>> prefs;
    std::vector<std::unique_ptr<AbsoluteMetric>> metrics;

    // Conducts a complete search (with backtracking) to find all schedule enrollments
    std::vector<std::pair<ScheduleEnrollment, Schedule>> FindAllSchedules() const {
        return {};
    }
};