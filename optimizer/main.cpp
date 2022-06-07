#include "order-[uniqname].hpp"


int main() {

  vector<unique_ptr<PreRequirement>> prereqs = order::PreReqs();

  for (auto& pair: courses) {
    set<uint16_t> badSections;
    for (auto const& sect: pair.second.sections) {
      for (auto const& req: prereqs) {
        if (not req->CheckSection(sect.second)) {
          badSections.insert(sect.first);
        }
      }
    }
    printf("in %s, %llu/%llu sections are bad\n", pair.first.c_str(), badSections.size(), pair.second.sections.size());
    auto new_end = remove_if(begin(pair.second.clusters), end(pair.second.clusters), [&](auto const& cluster) {
      return any_of(begin(cluster), end(cluster), [&](auto it) { return badSections.find(it) != end(badSections); });
    });
    pair.second.clusters.erase(new_end, end(pair.second.clusters));
    for (auto section: badSections) {
      pair.second.sections.erase(section);
    }
    printf("in %s, %llu clusters remain\n", pair.first.c_str(), pair.second.clusters.size());
  }
  Search search(courses);

  order::SetupSearch(search);

  SearchResults res = search.FindAllSchedules();
  for (size_t i = 0; i < res.course_order.size(); ++i) {
    if (i != 0) printf(",");
    printf("%s", res.course_order[i].c_str());
  }
  puts("");
  res.ForEachSchedule([&](ScheduleStats stats, auto const& stack) {
    for (size_t i = 0; i < stack.size(); ++i) {
      if (i != 0) printf(",");
      auto const& cluster = courses[res.course_order[i]].clusters[stack[i]];
      for (size_t j = 0; j < cluster.size(); ++j) {
        if (j != 0) printf(":");
        printf("%u", cluster[j]);
      }
    }
    printf(",%.6f,%.2f\n", stats.pref_score, stats.metrics[0]);
  });
  fflush(stdout);
}