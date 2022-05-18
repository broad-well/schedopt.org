import dayjs from "dayjs";

export function parseMeeting({ Days, Times }) {
  if (Days === "TBA" || Times === "TBA")
    return null;
  const dayNames = ["Mo", "Tu", "We", "Th", "Fr"];
  const [start, end] = Times.split(" - ").map((s) => dayjs(s, "h:mmA"));
  return {
    days: dayNames.map((name) => Days.includes(name) ? 1 : 0),
    startTime: [start.hour(), start.minute()],
    endTime: [end.hour(), end.minute()],
  };
}

export function meetingsOverlap(m1, m2) {
    if (m1 == null || m2 == null) return false;
    for (let day = 0; day < 5; ++day) {
      if (m1.days[day] && m2.days[day]) {
        const start1 = m1.startTime[0] * 60 + m1.startTime[1];
        const end1 = m1.endTime[0] * 60 + m1.endTime[1];
        const start2 = m2.startTime[0] * 60 + m2.startTime[1];
        const end2 = m2.endTime[0] * 60 + m2.endTime[1];
        const between = (a, less, more) => a > less && a < more;
        if (start1 === start2) return true;
        if (end1 === end2) return true;
        if (
          between(start1, start2, end2) || between(end1, start2, end2) ||
          between(start2, start1, end1) || between(end2, start1, end1)
        ) {
          return true;
        }
      }
    }
    return false;
  }