import { parseMeeting } from "./datetime.js";
// section number, type, creditHours, classNumber, meetings (parsed)

function arrayify(item) {
  return item instanceof Array ? item : [item];
}

export function compact(section) {
  return {
    sectionNumber: parseInt(section.SectionNumber),
    sectionType: section.SectionType,
    creditHours: section.CreditHours,
    classNumber: section.ClassNumber,
    meetings: arrayify(section.Meeting).map(parseMeeting),
    ... (section.ClassInstructors == undefined ? {} : {instructors: arrayify(section.ClassInstructors).map(i => i.InstrUniqname)})
  };
}
