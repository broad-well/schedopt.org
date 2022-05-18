import dayjs from "dayjs";
import customParseFormat from "dayjs/plugin/customParseFormat.js";
import { parseMeeting, meetingsOverlap } from "./datetime.js";
dayjs.extend(customParseFormat);

function ensureParsedMeetings(section) {
  if (section.meetings === undefined) {
    if (!(section.Meeting instanceof Array)) {
      section.Meeting = [section.Meeting];
    }
    section.meetings = section.Meeting.map(parseMeeting);
  }
  return section;
}

function sectionsOverlap(sec1, sec2) {
  ensureParsedMeetings(sec1);
  ensureParsedMeetings(sec2);
  for (const meet1 of sec1.meetings) {
    for (const meet2 of sec2.meetings) {
      if (meetingsOverlap(meet1, meet2)) {
        return true;
      }
    }
  }
  return false;
}

export function allClusters(sections) {
  const sectionMap = Object.fromEntries(sections.map(sect => [sect.SectionNumber, sect]));
  const components = new Set(sections.map((s) => s.SectionType));
  // Ignore midterm exams
  components.delete("MID");

  // Precondition: prefix is not empty
  function findAllClustersWithEnrollment(prefix) {
    if (prefix.length === components.size) {
      return [prefix.map((s) => s.toString()).sort()];
    }
    const remainingComponents = [...components].filter((it) =>
      !prefix.some((p) => it === sectionMap[p].SectionType)
    );
    const comp = remainingComponents[0];
    const nextSections = sections
      .filter((s) =>
        s.SectionType === comp &&
        s.AssociatedClass === sectionMap[prefix[0]].AssociatedClass
      ).map(s => s.SectionNumber);
    // Special case: not all components required (PSYCH111:060)
    if (nextSections.length === 0) {
      return [prefix.map((s) => s.toString()).sort()];
    }
    return nextSections
      .filter((nextSection) =>
        !prefix.some((existing) => sectionsOverlap(sectionMap[existing], sectionMap[nextSection]))
      )
      .flatMap((nextSection) =>
        findAllClustersWithEnrollment(prefix.concat([nextSection]))
      );
  }

  // If someone chose to enroll in this section, what other sections would they automatically enroll in?
  function buildEnrollPrefix(enroll) {
    const out = [enroll.SectionNumber];
    if (enroll.AutoEnrollSection1 != null) {
      // loose equal: "950" == 950
      // ok wtf ENGR100:101 (auto enroll section has a conflict)
      const cand = sections.filter((s) =>
        s.SectionNumber == enroll.AutoEnrollSection1 &&
        components.has(s.SectionType) &&
        !sectionsOverlap(enroll, s)
      );
      if (cand.length > 0) {
        out.push(cand[0].SectionNumber);
      }
    }
    if (enroll.AutoEnrollSection2 != null) {
      const cand = sections.filter((s) =>
        s.SectionNumber == enroll.AutoEnrollSection2 &&
        components.has(s.SectionType) &&
        out.every(existing => !sectionsOverlap(sectionMap[existing], s))
      );
      if (cand.length > 0) {
        out.push(cand[0].SectionNumber);
      }
    }
    return out;
  }

  // enrollment classes
  return new Set(
    sections.filter((s) => s.EnrollmentClassType === "E").flatMap((ec) =>
      findAllClustersWithEnrollment(buildEnrollPrefix(ec))
    ),
  );
}
