
<div id="global-container">
    <aside>
        <section id="graphs">
            <CanvasPlot height={denseGraphs ? 80 : 100} series={schedules.map(s => s.compositeScore)} currentIndex={scheduleIndex}>
                Composite preference score
            </CanvasPlot>
            <!-- <CurrentPlot dataset={schedules} x={s => s.metrics[0]} y="compositeScore" thisIndex={0} caption='Scatterplot of travel distance vs composite score' type='scatter' /> -->
            <!-- <CurrentPlot dataset={schedules} x="compositeScore" thisIndex={scheduleIndex} caption='Composite preference score compared to other schedules' /> -->
            {#each preferenceFactors as prefLabel, i}
                <CanvasPlot height={denseGraphs ? 80 : 100} series={schedules.map(s => s.preferences[i])} currentIndex={scheduleIndex}>
                    {prefLabel + ' preference factor'}
                </CanvasPlot>
            {/each}
            {#each metrics as metric, i}
                <CanvasPlot height={denseGraphs ? 80 : 100} series={schedules.map(s => s.metrics[i])} currentIndex={scheduleIndex}>
                    {metric + ' compared to other schedules'}
                </CanvasPlot>
            {/each}
            <div class="big">
                {#key {scatterX, scatterY}}
                <CanvasPlot height={(denseGraphs ? 180 : 240)} series={schedules}
                        x={s => JSON.parse(scatterX).reduce((obj, key) => obj[key], s)}
                        y={s => JSON.parse(scatterY).reduce((obj, key) => obj[key], s)} type='scatterplot'
                        currentIndex={scheduleIndex}>
                    Scatterplot of <select bind:value={scatterY}>
                        <option value={JSON.stringify(['compositeScore'])}>Composite preference</option>
                        {#each preferenceFactors as prefLabel, i}
                            <option value={JSON.stringify(['preferences', i])}>{prefLabel}</option>
                        {/each}
                        {#each metrics as metric, i}
                            <option value={JSON.stringify(['metrics', i])}>{metric}</option>
                        {/each}
                    </select> (y) vs. <select bind:value={scatterX}>
                        <option value={JSON.stringify(['compositeScore'])}>Composite preference</option>
                        {#each preferenceFactors as prefLabel, i}
                            <option value={JSON.stringify(['preferences', i])}>{prefLabel}</option>
                        {/each}
                        {#each metrics as metric, i}
                            <option value={JSON.stringify(['metrics', i])}>{metric}</option>
                        {/each}
                    </select> (x)
                </CanvasPlot>
                {/key}
            </div>
        </section>
        <div id="schedule-label">
            <h1 style="margin: 0">Schedule {scheduleIndex + 1} / {schedules.length}</h1>
            <span style="text-align:right;padding:0.25rem">
                <button on:click={goToPrev}>&leftarrow; Previous</button>
                <button on:click={goToNext}>Next &rightarrow;</button>
            </span>
        </div>
    </aside>
    <main>
        {#each kTimes as time, i}
            <div class="time" style="grid-row: {i*2+2}; grid-column: 1/7">{time}</div>
        {/each}
        {#each kWeekdayNames as weekday, i}
            <div class="weekday-name" style="grid-column: {i + 2}; grid-row: 1">{weekday}</div>
        {/each}
        {#if schedule != undefined}
            {#each schedule.clusters as sections, clusterIndex}
                {#each sections as section}
                    {#each courseDb[courseOrder[clusterIndex]][section].meetings as meeting}
                        {#each meeting.days as dayYes, dayNum}
                            {#if dayYes > 0}
                                <div class="time-block"
                                    style="grid-row: {timeBlockRows(meeting.startTime, meeting.endTime)}; grid-column: {dayNum+2}; background-color: {kColorPalette[clusterIndex % kColorPalette.length]}">
                                    <strong>{courseOrder[clusterIndex]}</strong> {courseDb[courseOrder[clusterIndex]][section.toString()].sectionType}<br>
                                    Section {padZeros(section)} &centerdot; <code>{courseDb[courseOrder[clusterIndex]][section.toString()].classNumber}</code>
                                </div>
                            {/if}
                        {/each}
                    {/each}
                {/each}
            {/each}
        {/if}
    </main>
</div>

<style>
    #schedule-label {
        display: flex;
        justify-content: space-between;
        align-items: center;
    }
    .time-block {
        border-radius: 6px;
        color: #fefefe;
        padding: 6px;
        font-size: calc(max(8px, min(16px, 0.92vw)));
        line-height: 0.9;
    }
    main {
        height: 100%;
        background-color: #efefef;
        border-radius: 1.5rem;
        box-sizing: border-box;
        gap: 2px;
        padding: 1rem;
        display: grid;
        grid-template: 16px repeat(28, 1fr) / 48px repeat(5, 1fr);
    }
    .time, .weekday-name {
        color: rgba(0, 0, 0, 0.7);
        font-size: 14px;
    }
    .time {
        line-height: 90%;
        border-top: 2px solid rgba(0, 0, 0, 0.2);
        margin-top: -2px; /* hack */
    }
    #global-container {
        position: relative;
        height: 100vh;
        width: 100vw;
        display: flex;
        padding: calc(min(2vw, 0.5rem));
        box-sizing: border-box;
        flex-direction: column;
    }
    #graphs {
        display: grid;
        grid-template-columns: 1fr;
        gap: 8px;
    }
    @media screen and (min-width: 900px) {
        #global-container {
            flex-direction: row;
        }
        aside {
            flex-basis: 40%;
        }
        main {
            flex-grow: 2;
            flex-basis: 60%;
        }
        #graphs {
            grid-template-columns: 1fr 1fr;
        }
        #graphs .big {
            grid-column: 1/3;
        }
    }
    aside {
        margin: 8px;
        display: flex;
        flex-direction: column;
        justify-content: space-between;
    }
    aside button {
        background-color: #5a5a5a;
        padding: 12px 24px;
        border: none;
        border-radius: 12px;
        color: white;
        cursor: pointer;
        box-shadow: 0 1px #969699;
        transition: all 0.1s ease;
        margin: 4px;
    }
    aside button:hover {
        box-shadow: 0 2px #969699;
        transform: translateY(-1px);
    }
    aside button:active {
        box-shadow: none;
        transform: translateY(2px);
    }
</style>

<script>
import { onMount } from "svelte";
    import CanvasPlot from "./CanvasPlot.svelte";
    const kColorPalette = ["rgb(91,88,143)", "rgb(66,134,33)", "rgb(106,39,134)", "rgb(115,123,85)", "rgb(42,43,240)", "rgb(169,104,28)", "rgb(215,37,163)", "rgb(36,128,161)"];
    const kTimes = ['7 AM', '8 AM', '9 AM', '10 AM', '11 AM', '12 PM', '1 PM', '2 PM', '3 PM', '4 PM', '5 PM', '6 PM', '7 PM', '8 PM']
    const kWeekdayNames = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri'];

    // {'EECS 280': {'1': {sectionDetails...}}, ...}
    export let courseDb;
    // ['EECS 280', 'EECS 203', ...]
    export let courseOrder;
    // ['Earliest time', 'Latest time']
    export let preferenceFactors;
    // ['Travel distance']
    export let metrics;
    // {clusters: [[5, 20], [3, 14], ...], compositeScore: <composite score>, preferences: [1, 0, 0.5, ...], metrics: [24003]}
    export let schedules = [];

    let scheduleIndex = 0;

    let scatterX = JSON.stringify(['metrics', 0]), scatterY = JSON.stringify(['compositeScore']);
    function captionScatter(path) {
        if (path[0] === 'compositeScore') return 'composite preference score';
        else if (path[0] === 'metrics') return metrics[path[1]];
        else if (path[0] === 'preferences') return preferenceFactors[path[1]];
    }

    $: denseGraphs = metrics.length + preferenceFactors.length > 3;
    $: schedule = schedules[scheduleIndex];


    function timeBlockRows(startTime, endTime) {
        const startMin = startTime[0] * 60 + startTime[1];
        const endMin = endTime[0] * 60 + endTime[1];
        // 420
        const startIndex = Math.max(0, Math.round((startMin - 420) / 30)) + 2;
        const endIndex = Math.min(Math.round((endMin - 420) / 30), 28) + 2;
        return `${startIndex}/${endIndex}`;
    }

    const goToNext = () => scheduleIndex = (scheduleIndex + 1) % schedules.length;
    const goToPrev = () => scheduleIndex = scheduleIndex === 0 ? (schedules.length - 1) : (scheduleIndex - 1);

    onMount(() => {
        window.addEventListener('keydown', ev => {
            if (ev.key === 'ArrowRight') goToNext();
            else if (ev.key === 'ArrowLeft') goToPrev();
        })
    })

    function padZeros(sectionNum) {
        if (typeof(sectionNum) === 'number') return padZeros(sectionNum.toString());
        if (typeof(sectionNum) === 'string' && sectionNum.length >= 3) return sectionNum;
        return padZeros('0' + sectionNum);
    }
</script>

