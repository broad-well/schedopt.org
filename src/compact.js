import { parseMeeting } from "./datetime.js";
// section number, type, creditHours, classNumber, meetings (parsed)

export function compact(section) {
  return {
    sectionNumber: parseInt(section.SectionNumber),
    sectionType: section.SectionType,
    creditHours: section.CreditHours,
    classNumber: section.ClassNumber,
    meetings:
      (section.Meeting instanceof Array ? section.Meeting : [section.Meeting])
        .map(parseMeeting),
  };
}
