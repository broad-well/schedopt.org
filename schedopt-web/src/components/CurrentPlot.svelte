{#key {dataset, x, y, thisIndex, caption}}
    <div id="hist-container" use:plot></div>
{/key}

<style>
    :global(#hist-container figure) {
        margin: 0 0;
    }
    #hist-container {
        background-color: #efefef;
        padding: 8px;
        border-radius: 8px;
        margin-bottom: 8px;
        font-size: 14px;
    }
</style>
<script>
    import * as Plot from "@observablehq/plot";
    import { onMount } from "svelte";

    /**
     * @type {any[]}
     */
    export let dataset;
    /**
     * @type {string | (s: any) => number}
    */
    export let x;
    /**
     * @type {string | (s: any) => number}
    */
    export let y;
    /**
     * @type {number}
     */
    export let thisIndex;
    /**
     * @type {string}
     */
    export let caption;
    /**
     * if "histogram", `y` unused
     * @type {"histogram"|"scatter"}
     */
    export let type = "histogram";

    const kPlotConfig = {
        height: 140,
        width: width,
        style: { 
            background: 'rgba(0, 0, 0, 0)',
            fontFamily: '"Source Sans 3", system-ui, sans-serif',
            fontSize: 12
        }
    };

    const plot = (container) => {
        const extractor = x instanceof Function ? x : s => s[x];
        if (type === 'histogram') {
            container.appendChild(Plot.plot({
                caption: caption,
                marks: [
                    Plot.rectY(dataset, Plot.binX({y: "count"}, {x: {thresholds: 40, value: x}, fill: 'rgba(100, 100, 100, 0.5)'})),
                    Plot.ruleX([extractor(dataset[thisIndex])], {stroke: 'rgba(0, 0, 0, 1)', strokeWidth: 3})
                ],
                ...kPlotConfig
            }))
        } else if (type === 'scatter') {
            container.appendChild(Plot.plot({
                caption: caption,
                marks: [
                    Plot.dot(dataset, {x, y,
                        stroke: s => dataset[thisIndex] === s ? 'rgba(0,0,0,1)' : 'rgba(100,100,100,0.5)',
                        fill: s => dataset[thisIndex] === s ? 'rgba(0,0,0,1)' : 'rgba(100,100,100,0.5)'})
                ],
                ...kPlotConfig
            }))
        }
    };
</script>