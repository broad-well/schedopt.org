import assert from 'assert';
import { marshalSection } from '../src/dynamo.js'

describe('marshalling and unmarshalling sections for DynamoDB', () => {
  describe('marshalSection', () => {
    it('should marshal single-meeting section correctly', () => {
      const marshalled = marshalSection({
        courseCode: 'ENGR 151',
        sectionNumber: 101,
        sectionType: 'DIS',
        creditHours: 4,
        classNumber: 14223,
        meetings: [{
          days: [0, 0, 1, 0, 0],
          startTime: [14, 0],
          endTime: [15, 0]
        }]
      });
      assert.deepEqual(marshalled, {
        CourseCode: {S: 'ENGR 151'},
        SectionNumber: {N: 101},
        SectionType: {S: 'DIS'},
        CreditHours: {N: 4},
        ClassNumber: {N: 14223},
        Meetings: {L: [
          {M: {
            days: {L: [{N: 0}, {N: 0}, {N: 1}, {N: 0}, {N: 0}]},
            startTime: {L: [{N: 14}, {N: 0}]},
            endTime: {L: [{N: 15}, {N: 0}]}
          }}
        ]}
      });
    });
  });
});