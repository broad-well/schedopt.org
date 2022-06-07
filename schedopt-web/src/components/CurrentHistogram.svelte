{#key {dataset, dataKey, thisIndex, caption}}
    <div id="hist-container" use:plot></div>
{/key}

<style>
    :global(#hist-container figure) {
        margin: 0 0;
    }
    #hist-container {
        background-color: #efefef;
        padding: 12px;
        border-radius: 8px;
        margin-bottom: 12px;
    }
</style>
<script>
    import * as Plot from "@observablehq/plot";
    import { width } from '@observablehq/stdlib/dist/stdlib';
    import { onMount } from "svelte";

    /**
     * @type {any[]}
     */
    export let dataset;
    /**
     * @type {string | (s: any) => number}
    */
    export let dataKey;
    /**
     * @type {number}
     */
    export let thisIndex;
    /**
     * @type {string}
     */
    export let caption;
    let container;

    $: console.log(dataset);

    const plot = (container) => {
        const extractor = dataKey instanceof Function ? dataKey : s => s[dataKey];
        container.appendChild(Plot.plot({
            caption: caption,
            marks: [
                Plot.rectY(dataset, Plot.binX({y: "count"}, {x: {thresholds: 40, value: dataKey}, fill: 'rgba(100, 100, 100, 0.5)'})),
                Plot.ruleX([extractor(dataset[thisIndex])], {stroke: 'rgba(0, 0, 0, 1)', strokeWidth: 3})
            ],
            height: 180,
            width: width,
            style: { 
                background: 'rgba(0, 0, 0, 0)',
                fontFamily: '"Source Sans 3", system-ui, sans-serif',
                fontSize: 12
            }
        }))
    };
</script>