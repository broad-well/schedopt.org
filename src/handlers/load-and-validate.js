import { compact } from '../compact.js';
import { Database } from '../dynamo.js';
import { allClusters } from '../genClusters.js';
import * as soc from '../socApi.js'
// high level procedure
// get SOC details about a course. First try reading from DB, or else get it from SOC API (getting/refreshing access token as needed)
// if no details available, return error (course invalid for this semester)
// if didn't read from DB, put this into the DB
// return ok (validated)

// const courses = [['EECS', 280], ['STATS', 412], ['ROB', 101], ['ENGLISH', 124], ['SPANISH', 232], ['ECON', 101]]
// Promise.all(courses.map(c => 
//     soc.fetchSections('AAIkNzEyMjI3MzYtNzNjMy00N2NlLWI4NWQtMDVkNzdjMDU0ZDViBztv5kodoJNig-9v7Ph_gB-BS559eIHAVCpscJJ8sQ-S43u2iJfHugPCAEkRALkAAl71rrvPQ-_7qe1iFu_nHvWFHt8quwn08X08_qKs18bzggcGT4bjx1NzBjbxu2XbAy9Yw9i1quuuvPGDpLqvOA', c)))
//     .then(console.log).catch(console.error)

class RequestMalformedException extends Error {
    constructor(msg, code) {
        super(msg);
        this.code = code ?? 422;
    }
}

class UnknownCourseException extends Error {
    constructor(course) {
        super(`Unknown course: ${course}`);
    }
}

function requireRequest(cond, error, code) {
    if (!cond) {
        throw new RequestMalformedException(error, code);
    }
}

const apiRetrievers = {
    umich: {
        FA22: async (db, course) => {
            let key = await db.getApiAccessKey();
            if (key == null) {
                key = await getSOCToken();
                await db.updateApiAccessKey(key, new Date(Date.now() + 1000 * 60 * 60));
            }
            const rawResponse = soc.fetchSections(key, 'FA22', course);
            if (!('getSOCSectionsResponse' in rawResponse)) {
                throw new UnknownCourseException(course);
            }
            const rawSections = rawResponse.getSOCSectionsResponse.Section;
            const sections = (rawSections instanceof Array ? rawSections : [rawSections]).map(compact);
            return sections;
        }
    }
};

/**
 * @param {Database} db Database object
 * @param {string} school 
 * @param {string} term 
 */
async function courseLoadAndValid(db, school, term, course) {
    if (await db.hasCourse(course)) return true;
    try {
        const sections = await apiRetrievers[school][term](db, course);
        await db.writeCourse(course, sections, Array.from(allClusters(sections)).map(cluster => cluster.map(section => parseInt(section))));
        return true;
    } catch (exc) {
        if (exc instanceof UnknownCourseException) return false;
        throw exc;
    }
}

exports.handler = async (event) => {
    // POST body format: {school: 'umich', term: 'FA22', courses: [['EECS',280],['EECS',203],['MATH',215]]}
    // response format: {status: 'ok'/'error', invalidCourses: [['EECS',204]]}
    try {
        requireRequest(event.httpMethod === 'POST', 'Incorrect method', 405);
        requireRequest(event.body != null && event.body.length > 0, 'Missing body');
        const body = JSON.parse(event.body);
        for (const key of ['school', 'term', 'courses']) {
            requireRequest(body[key] != undefined, `Empty or invalid value for "${key}"`);
        }
        requireRequest(Object.keys(apiRetrievers).includes(body.school), `Unknown school: "${body.school}`);
        requireRequest(Object.keys(apiRetrievers[body.school]).includes(body.term), `Unknown term for ${body.school}: "${body.term}"`);
        

        const db = new Database(body.school, body.term);
        await db.createTablesIfNeeded();
        const badCourses = (await Promise.all(body.courses.map(course => [course, courseLoadAndValid(db, body.school, body.term, course)])))
            .filter(it => !it[1]).map(it => it[0])
        
        return {
            statusCode: 200,
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({status: badCourses.length === 0 ? 'ok' : 'error', invalidCourses: badCourses})
        };
    } catch (exc) {
        console.error(exc);
        return {
            statusCode: 500,
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(exc)
        }
    }
};