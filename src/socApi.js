import axios from "axios";
import { umSocApi } from './secret.js';

export async function requestAccessToken() {
  const res = await axios({
      url: "https://apigw.it.umich.edu/um/aa/oauth2/token",
      method: 'post',
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
      },
      data:
        `grant_type=client_credentials&client_id=${umSocApi.clientId}&client_secret=${umSocApi.secret}&scope=umscheduleofclasses`,
    },
  );
  return res.data.access_token;
}

export async function fetchSections(token, term, [catalog, number]) {
  const termMap = {
    FA22: 2410
  };
  const res = await axios.get(
    `https://apigw.it.umich.edu/um/Curriculum/SOC/Terms/${termMap[term]}/Schools/UM/Subjects/${catalog}/CatalogNbrs/${number}/Sections?IncludeAllSections=Y`,
    {
      headers: {
        "Authorization": "Bearer " + token,
        "X-IBM-Client-Id": umSocApi.clientId,
        "Accept": "application/json",
      },
    },
  );
  return res.data;
}
