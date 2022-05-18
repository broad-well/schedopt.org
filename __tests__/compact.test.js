import assert from "assert";
import { compact } from "../src/compact.js";

describe("compact", () => {
  it("should transcribe single-meeting EECS 376 discussion correctly", () => {
    const section = {
      "SectionNumber": "011",
      "SectionType": "DIS",
      "SectionTypeDescr": "Discussion",
      "SessionDescr": "Regular",
      "InstructionMode": "In Person",
      "ClassTopic": null,
      "EnrollmentStatus": "Wait List",
      "EnrollmentTotal": 40,
      "EnrollmentCapacity": 40,
      "AvailableSeats": 0,
      "WaitTotal": 18,
      "WaitCapacity": 9999,
      "CreditHours": 4,
      "CombinedSectionID": null,
      "ClassNumber": 10397,
      "AutoEnrollSection1": null,
      "AutoEnrollSection2": null,
      "AssociatedClass": 1,
      "EnrollmentClassType": "E",
      "Meeting": {
        "MeetingNumber": 1,
        "Days": "Fr",
        "Times": "9:30AM - 10:30AM",
        "ClassMtgTopic": "TBA",
        "Instructors": "Staff",
      },
    };
    assert.deepEqual(compact(section), {
      sectionNumber: 11,
      sectionType: "DIS",
      creditHours: 4,
      classNumber: 10397,
      meetings: [{
        days: [0, 0, 0, 0, 1],
        startTime: [9, 30],
        endTime: [10, 30],
      }],
    });
  });

  it("should transcribe multiple-meeting CHEM 130 lecture correctly", () => {
    const section = {
      "SectionNumber": 400,
      "SectionType": "LEC",
      "SectionTypeDescr": "Lecture",
      "SessionDescr": "Regular",
      "InstructionMode": "In Person",
      "ClassTopic": null,
      "EnrollmentStatus": "Open",
      "EnrollmentTotal": 30,
      "EnrollmentCapacity": 100,
      "AvailableSeats": 70,
      "WaitTotal": 0,
      "WaitCapacity": 0,
      "CreditHours": 3,
      "CombinedSectionID": null,
      "ClassNumber": 10820,
      "AutoEnrollSection1": null,
      "AutoEnrollSection2": null,
      "AssociatedClass": 4,
      "EnrollmentClassType": "N",
      "Meeting": [{
        "MeetingNumber": 1,
        "Days": "MoWe",
        "Times": "10:00AM - 11:00AM",
        "ClassMtgTopic": "TBA",
        "Instructors": "Castaneda,Carol Ann",
      }, {
        "MeetingNumber": 2,
        "Days": "Fr",
        "Times": "10:00AM - 12:00PM",
        "ClassMtgTopic": "TBA",
        "Instructors": "Castaneda,Carol Ann",
      }],
      "ClassInstructors": {
        "InstrUniqname": "CASTANCA",
        "InstrName": "Castaneda,Carol Ann",
      },
    };
    assert.deepEqual(compact(section), {
      sectionNumber: 400,
      sectionType: "LEC",
      creditHours: 3,
      classNumber: 10820,
      meetings: [
        {
          days: [1, 0, 1, 0, 0],
          startTime: [10, 0],
          endTime: [11, 0]
        },
        {
          days: [0, 0, 0, 0, 1],
          startTime: [10, 0],
          endTime: [12, 0]
        }
      ],
    });
  });
});
