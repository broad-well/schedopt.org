import {
  BatchWriteItemCommand,
  CreateTableCommand,
  DynamoDBClient,
  GetItemCommand,
  PutItemCommand,
  QueryCommand,
  ResourceInUseException,
} from "@aws-sdk/client-dynamodb";

/*
Sections-umich-fa22:
    CourseCode: String, catalog + " " + courseNumber
    SectionNumber: Number,
    SectionType: String (enum),
    CreditHours: Number,
    ClassNumber: Number,
    Meetings: {days: Number[], startTime: Number[], endTime: Number[]}[]

Clusters-umich-fa22:
    CourseCode: String
    Clusters: Number[][]
 */

function courseToCode(catalog, number) {
  return catalog.toUpperCase() + " " + number.toString();
}

export function unmarshalSection(section) {
  return {
    courseCode: section.CourseCode.S,
    sectionNumber: section.SectionNumber.N,
    sectionType: section.SectionType.S,
    creditHours: section.CreditHours.N,
    classNumber: section.ClassNumber.N,
    meetings: section.Meetings.L.map((item) =>
      Object.fromEntries(
        Object.entries(item.M).map(([k, v]) => [k, v.L.map((s) => s.N)]),
      )
    ),
  };
}

export function marshalSection(section, courseCode) {
  return {
    CourseCode: { S: courseCode },
    SectionNumber: { N: section.sectionNumber },
    SectionType: { S: section.sectionType },
    CreditHours: { N: section.creditHours },
    ClassNumber: { N: section.classNumber },
    Meetings: {
      L: section.meetings.map(({ days, startTime, endTime }) => (
        {
          M: {
            days: { L: days.map((day) => ({ N: day })) },
            startTime: { L: startTime.map((num) => ({ N: num })) },
            endTime: { L: endTime.map((num) => ({ N: num })) },
          },
        }
      )),
    },
  };
}

export class Database {
  constructor(school, term) {
    this.school = school;
    this.term = term;
    this.client = new DynamoDBClient();
  }

  async createTablesIfNeeded() {
    try {
      await this.client.send(
        new CreateTableCommand({
          AttributeDefinitions: [
            { AttributeName: "CourseCode", AttributeType: "S" },
            { AttributeName: "SectionNumber", AttributeType: "N" },
            { AttributeName: "SectionType", AttributeType: "S" },
            { AttributeName: "CreditHours", AttributeType: "N" },
            { AttributeName: "ClassNumber", AttributeType: "N" },
            { AttributeName: "Meetings", AttributeType: "B" },
          ],
          KeySchema: [
            { AttributeName: "CourseCode", KeyType: "HASH" },
            { AttributeName: "SectionNumber", KeyType: "RANGE" },
          ],
          ProvisionedThroughput: {
            ReadCapacityUnits: 2,
            WriteCapacityUnits: 1,
          },
          TableName: this._sectionTableName(),
        }),
      );
      await this.client.send(
        new CreateTableCommand({
          AttributeDefinitions: [
            { AttributeName: "CourseCode", AttributeType: "S" },
            { AttributeName: "Clusters", AttributeType: "B" },
          ],
          KeySchema: [
            { AttributeName: "CourseCode", KeyType: "HASH" },
          ],
          TableName: this._clusterTableName(),
        }),
      );
      await this.client.send(new CreateTableCommand({
        AttributeDefinitions: [
          { AttributeName: "School", AttributeType: "S" },
          { AttributeName: "AccessKey", AttributeType: "S" },
          { AttributeName: "AccessExpiration", AttributeType: "S" }
        ],
        KeySchema: [
          { AttributeName: 'School', KeyType: 'HASH' }
        ],
        TableName: 'APIKeys'
      }));
    } catch (exc) {
      // Expecting RIUE if tables already exist
      if (!(exc instanceof ResourceInUseException)) throw exc;
    }
  }

  async getApiAccessKey() {
    const item = await this.client.send(new GetItemCommand({
      TableName: 'APIKeys',
      Key: {
        School: { S: this.school },
      },
    }));
    if (item.Item == undefined) return null;
    if (new Date(item.Item.AccessExpiration.S) > new Date()) return null;
    return item.Item.AccessKey.S;
  }

  async updateApiAccessKey(newKey, expireDate) {
    await this.client.send(new PutItemCommand({
      TableName: 'APIKeys',
      Item: {
        School: {S: this.school},
        AccessKey: {S: newKey},
        AccessExpiration: {S: expireDate.toISOString()}
      }
    }));
  }

  async hasCourse([catalog, number]) {
    const res = await this.client.send(
      new QueryCommand({
        KeyConditionExpression: "CourseCode = :c",
        ExpressionAttributeValues: {
          ":c": { S: courseToCode(catalog, number) },
        },
        ProjectionExpression: "SectionNumber",
        TableName: this._sectionTableName(),
      }),
    );
    return res.Count > 0;
  }
  async fetchSections([catalog, number]) {
    const res = await this.client.send(
      new QueryCommand({
        KeyConditionExpression: "CourseCode = :c",
        ExpressionAttributeValues: {
          ":c": { S: courseToCode(catalog, number) },
        },
        TableName: this._sectionTableName(),
      }),
    );
    return res.Items.map(unmarshalSection);
  }
  async writeCourse([catalog, number], sections, clusters) {
    const courseCode = courseToCode(catalog, number);
    await this.client.send(
      new BatchWriteItemCommand({
        RequestItems: {
          [this._sectionTableName()]: sections.map((section) => ({
            PutRequest: {
              Item: marshalSection(section, courseCode),
            },
          })),
          [this._clusterTableName()]: [{
            PutRequest: {
              Item: {
                CourseCode: { S: courseCode },
                Clusters: {
                  L: clusters.map((cluster) => ({
                    L: cluster.map((section) => ({ N: section })),
                  })),
                },
              },
            },
          }],
        },
      }),
    );
  }

  _sectionTableName() {
    return `Sections-${this.school}-${this.term}`;
  }
  _clusterTableName() {
    return `Clusters-${this.school}-${this.term}`;
  }
}
