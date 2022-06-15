<script>
    import ScheduleSetViewer from "../components/ScheduleSetViewer.svelte";

    function parseUniqnameFromUrl() {
        const url = new URL(window.location.href)
        return url.searchParams.get('u')
    }
    async function loadScheduleSet() {
        const id = parseUniqnameFromUrl() ?? prompt('Enter your uniqname');
        const [dbJson, combsCsv] = [`/${id}-db.json`, `/${id}.csv`];
        const [dbRes, combsRes] = (await Promise.all([fetch(dbJson), fetch(combsCsv)]));
        const db = await dbRes.json();
        const combs = await combsRes.text();
        // multiple course-combination support. 2 empty lines to designate the boundary between two course combinations
        const courseCombs = combs.split('\n\n\n');
        const sections = courseCombs.flatMap((combs, i) => {
            const cRows = combs.split('\n').filter(i => i.length > 0);
            const headerRow = cRows[0].split(',');
            const courseOrder = headerRow.filter(it => !it.includes('::'));
            const prefIndexes = headerRow.map((s, i) => [s, i]).filter(it => it[0].startsWith('pref::')).map(it => [it[0].substring(6), it[1]]);
            const metricIndexes = headerRow.map((s, i) => [s, i]).filter(it => it[0].startsWith('metric::')).map(it => [it[0].substring(8), it[1]]);
            const compositeIndex = headerRow.indexOf('prefs::compositeScore');
            return cRows.slice(1).map(comb => {
                const tokens = comb.split(',');
                const clusters = tokens.slice(0, courseOrder.length).map(s => s.split(':').map(i => parseInt(i)));
                const compositeScore = tokens[compositeIndex];
                const metrics = metricIndexes.map(s => Number(tokens[s[1]]));
                const prefFactors = prefIndexes.map(s => Number(tokens[s[1]]));
                return {clusters, compositeScore: Number(compositeScore), metrics, preferences: prefFactors, courseOrder, courseOrderIndex: i};
            });
        });
        const firstRowTokens = combs.substring(0, combs.indexOf('\n')).split(',');
        return {
            courseDb: db,
            preferenceFactors: firstRowTokens.filter(it => it.startsWith('pref::')).map(s => s.substring(6)),
            metrics: firstRowTokens.filter(it => it.startsWith('metric::')).map(s => s.substring(8)),
            schedules: sections.sort((a, b) => {
                return b.compositeScore - a.compositeScore;
            })
        };
    }
</script>
<svelte:head>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="anonymous">
    <link href="https://fonts.googleapis.com/css2?family=Source+Sans+3:wght@400;700&display=swap" rel="stylesheet">
    <title>SchedOpt Schedule Viewer</title>
</svelte:head>

{#await loadScheduleSet()}
  <div style="margin: 2rem">
      <h1>Loading your schedules...</h1>
  </div>
{:then params}
    <ScheduleSetViewer {...params} />
{:catch err}
    {err}
    {(() => { alert("I haven't generated your schedule options. Please tell me your desired courses and scheduling preferences via form.schedopt.org."); return ''; })()}
{/await}

<style lang="scss">
    :global(body) {
        margin: 0 0;
        background-color: #d7d7d9;
        font-family: 'Source Sans 3', sans-serif;
    }
</style>
