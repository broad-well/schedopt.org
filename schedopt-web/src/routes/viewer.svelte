<script>
    import ScheduleSetViewer from "../components/ScheduleSetViewer.svelte";

    function parseUniqnameFromUrl() {
        const url = new URL(window.location.href)
        return url.searchParams.get('u')
    }
    async function loadScheduleSet() {
        const id = parseUniqnameFromUrl() ?? prompt('enter your uniqname');
        const [dbJson, combsCsv] = [`/${id}-db.json`, `/${id}.csv`];
        const [dbRes, combsRes] = (await Promise.all([fetch(dbJson), fetch(combsCsv)]));
        const db = await dbRes.json();
        const combs = await combsRes.text();
        const cRows = combs.split('\n').slice(0, -1);
        const courseOrder = cRows[0].split(',');
        const sections = cRows.slice(1).map(comb => {
            const tokens = comb.split(',');
            const clusters = tokens.slice(0, courseOrder.length).map(s => s.split(':').map(i => parseInt(i)));
            const compositeScore = tokens[courseOrder.length];
            const metrics = tokens.slice(courseOrder.length + 1).map(s => Number(s));
            return {clusters, compositeScore: Number(compositeScore), metrics};
        });
        return {
            courseDb: db,
            courseOrder,
            preferenceFactors: ['???'],
            metrics: ['Travel distance (m)'],
            schedules: sections.sort((a, b) => {
                const scoreDiff = b.compositeScore - a.compositeScore;
                if (scoreDiff !== 0) return scoreDiff;
                return a.metrics[0] - b.metrics[0];
            })
        };
    }
</script>
<svelte:head>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="anonymous">
    <link href="https://fonts.googleapis.com/css2?family=Source+Sans+3:wght@400;700&display=swap" rel="stylesheet">
</svelte:head>

{#await loadScheduleSet()}
 {""}
{:then params}
    <ScheduleSetViewer {...params} />
{:catch err}
    {alert('No schedule found for you. Sorry')}
{/await}

<style lang="scss">
    :global(body) {
        margin: 0 0;
        background-color: #d7d7d9;
        font-family: 'Source Sans 3', sans-serif;
    }
</style>
