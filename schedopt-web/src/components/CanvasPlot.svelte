<figure class="plot-container" {style}>
    {#key {currentIndex, type}}
        <canvas use:populate style="height: {height}px" bind:this={canvas}></canvas>
    {/key}
    <figcaption><slot /></figcaption>
</figure>
<style>
    .plot-container {
        background-color: #efefef;
        padding: 8px;
        border-radius: 8px;
        font-size: 14px;
        margin: 0;
    }
    .plot-container canvas {
        width: 100%;
    }
</style>
<script>
import { onMount } from "svelte";

    export let style;
    export let rectFill = 'rgba(100,100,100,0.5)';
    export let binCount = 40;
    export let series = [];
    export let height = 100;
    export let highlightColor = 'black';//'#00274C';
    /**
     * @type {'histogram'|'scatterplot'}
     */
    export let type = 'histogram';
    export let currentIndex;
    export let x, y; // only for scatterplot: extractors
    let barImage = null;
    let canvas;

    const bottomMargin = 16;
    const horizMargin = 18;

    function makeGraph(width, height, series, x, y) {
        const obj = {
            histogram: Histogram,
            scatterplot: Scatterplot
        };
        return new (obj[type])(width, height, series, x, y);
    }

    function populate(elem) {
        /**
         * @type {CanvasRenderingContext2D}
         */
        const ctx = elem.getContext('2d');
        const dpr = (window.devicePixelRatio || 1) * 2;
        const {width, height} = elem.getBoundingClientRect();
        elem.style.height = height + 'px';
        elem.width = width * dpr;
        elem.height = height * dpr;
        ctx.scale(dpr, dpr);
        
        const graph = makeGraph(width, height, series, x, y);
        if (barImage === null || barImage.width !== width || barImage.height !== height) {
            graph.drawGraph(ctx, width, height);
            barImage = {data: ctx.getImageData(0, 0, width*dpr, height*dpr), width, height};
        } else {
            ctx.putImageData(barImage.data, 0, 0);
        }
        
        if (currentIndex !== undefined) {
            graph.highlightCurrent(ctx, width, height, currentIndex);
        }
    }

    onMount(() => {
        window.addEventListener('resize', ev => {
            console.log('resizing');
            populate(canvas);
        });
    });

    class Histogram {
        constructor(width, height, series) {
            const {min, max} = minmax(series);
            this.globalMin = min;
            this.globalMax = max;
            this.insufficient = series.length < 6 || minMaxTooClose(min, max);
            if (!this.insufficient) {
                this.binSize = (this.globalMax - this.globalMin) / binCount;
                this.binWidth = (width - 2 * horizMargin) / binCount;
                this.rectYMax = height - bottomMargin;
                this.binWeights = new Array(binCount).fill(0);
                for (const num of series) {
                    this.binWeights[Math.min(this.binWeights.length - 1, Math.floor((num - this.globalMin) / this.binSize))]++;
                }
                this.maxBinWeight = this.binWeights.reduce((a, b) => Math.max(a, b), 0);
                this.binHeightScale = this.rectYMax / this.maxBinWeight;
            }
        }

        drawGraph(ctx, width, height) {
            if (this.insufficient) {
                drawInsufficientData(ctx, width, height);
                return;
            }
            ctx.fillStyle = rectFill;
            for (let i = 0; i < this.binWeights.length; ++i) {
                const rectHeight = this.binHeightScale * this.binWeights[i];
                ctx.fillRect(horizMargin + this.binWidth * i, this.rectYMax - rectHeight, this.binWidth, rectHeight);
            }
            drawHorizTicks(ctx, horizMargin, width - horizMargin, this.rectYMax, this.globalMin, this.globalMax);
        }

        highlightCurrent(ctx, width, height, currentIndex) {
            if (this.insufficient) {
                ctx.fillStyle = '#555555';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.font = '12px "Source Sans 3", system-ui, sans-serif';
                ctx.fillText(`Value: ${expressNumber(series[currentIndex])}`, width/2, height/2 + 16);
                return;
            }
            const mx = (series[currentIndex] - this.globalMin) / (this.globalMax - this.globalMin) * (width - 2 * horizMargin) + horizMargin;
            ctx.fillStyle = highlightColor;
            ctx.fillRect(mx - 1, 0, 2, height);
        }
    }

    class Scatterplot {
        constructor(width, height, series, x, y) {
            this.bottomMargin = 18;
            this.rightMargin = 16;
            this.topMargin = 12;
            this.leftMargin = 36;
            this.xs = series.map(x);
            this.ys = series.map(y);
            const {min: xmin, max: xmax} = minmax(this.xs);
            const {min: ymin, max: ymax} = minmax(this.ys);
            this.ymin = ymin;
            this.ymax = ymax;
            this.xmin = xmin;
            this.xmax = xmax;
            this.projectX = x => (x - xmin) / (xmax - xmin) * (width - this.leftMargin - this.rightMargin) + this.leftMargin;
            this.projectY = y => height - this.bottomMargin - ((y - ymin) / (ymax - ymin) * (height - bottomMargin - this.topMargin));
            this.insufficient = series.length < 4 || minMaxTooClose(xmin, xmax) || minMaxTooClose(ymin, ymax);
        }

        drawGraph(ctx, width, height) {
            if (this.insufficient) {
                drawInsufficientData(ctx, width, height);
                return;
            }
            ctx.fillStyle = 'rgba(100,100,100,0.3)';
            for (let i = 0; i < series.length; ++i) {
                const x = this.projectX(this.xs[i]), y = this.projectY(this.ys[i]);
                ctx.fillRect(x-1.25, y-1.25, 2.5, 2.5);
            }
            drawHorizTicks(ctx, this.leftMargin, width - this.rightMargin, height - this.bottomMargin, this.xmin, this.xmax);
            drawVertTicks(ctx, this.topMargin, height - this.bottomMargin, this.leftMargin, this.ymin, this.ymax);
        }

        highlightCurrent(ctx, width, height, index) {
            if (this.insufficient) {
                ctx.fillStyle = '#555555';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.font = '12px "Source Sans 3", system-ui, sans-serif';
                ctx.fillText(`(${expressNumber(this.xs[index])}, ${expressNumber(this.ys[index])})`, width/2, height/2 + 16);
                return;
            }
            ctx.fillStyle = highlightColor;
            const x = this.projectX(this.xs[index]);
            const y = this.projectY(this.ys[index]);
            ctx.fillRect(x - 3, y - 3, 6, 6);
        }
    }

    function drawInsufficientData(ctx, width, height) {
        ctx.fillStyle = 'black';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.font = '16px "Source Sans 3", system-ui, sans-serif';
        ctx.fillText('Insufficient variance for graph', width/2, height/2);
        return;
    }

    function minmax(series) {
        return {
            min: series.reduce((a, b) => Math.min(a, b), 1e100),
            max: series.reduce((a, b) => Math.max(a, b), 0)
        };
    }
    function drawHorizTicks(ctx, minX, maxX, y, minVal, maxVal, tickCount = 5) {
        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        ctx.font = '12px "Source Sans 3", system-ui, sans-serif';
        ctx.fillStyle = 'black';
        for (let i = 0; i < tickCount; ++i) {
            const cx = minX + (i / (tickCount - 1) * (maxX - minX));
            ctx.fillRect(cx-0.5, y, 1, 4);
            ctx.fillText(expressNumber((maxVal - minVal) * (i / (tickCount - 1)) + minVal), cx, y + 5);
        }
    }
    function drawVertTicks(ctx, minY, maxY, x, minVal, maxVal, tickCount = 5) {
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        ctx.font = '12px "Source Sans 3", system-ui, sans-serif';
        ctx.fillStyle = 'black';
        for (let i = 0; i < tickCount; ++i) {
            const cy = maxY - (i / (tickCount - 1) * (maxY - minY));
            ctx.fillRect(x-4, cy-0.5, 4, 1);
            ctx.fillText(expressNumber((maxVal - minVal) * (i / (tickCount - 1)) + minVal), x - 5, cy);
        }
    }

    function minMaxTooClose(min, max) {
        return (max - min) < ((min + max) / 2) * 0.001;
    }

    // /**
    //  * 
    //  * @param {CanvasRenderingContext2D} ctx
    //  * @param width
    //  * @param height
    //  */
    // function drawScatterplot(ctx, width, height) {
       
    // }

    // function drawHistogram(ctx, width, height) {
    //     // Generate histogram
    //     const {min: globalMin, max: globalMax} = minmax(series);
    //     if (series.length < 6 || globalMin >= globalMax) {
    //         ctx.strokeStyle = 'black';
    //         ctx.textAlign = 'center';
    //         ctx.textBaseline = 'middle';
    //         ctx.font = '16px "Source Sans 3", system-ui, sans-serif';
    //         ctx.fillText('Insufficient data', width/2, height/2);
    //         return;
    //     }

    //     const binSize = (globalMax - globalMin) / binCount;
    //     const binWidth = (width - 2 * horizMargin) / binCount;
    //     const rectYMax = height - bottomMargin;
    //     const binWeights = new Array(binCount).fill(0);
    //     for (const num of series) {
    //         binWeights[Math.min(binWeights.length - 1, Math.floor((num - globalMin) / binSize))]++;
    //     }
    //     const maxBinWeight = binWeights.reduce((a, b) => Math.max(a, b), 0);
    //     const binHeightScale = rectYMax / maxBinWeight;
    //     ctx.fillStyle = rectFill;
    //     for (let i = 0; i < binWeights.length; ++i) {
    //         const rectHeight = binHeightScale * binWeights[i];
    //         ctx.fillRect(horizMargin + binWidth * i, rectYMax - rectHeight, binWidth, rectHeight);
    //     }

    //     // X ticks
    //     drawHorizTicks(ctx, horizMargin, width - horizMargin, rectYMax, globalMin, globalMax);
    // }

    function expressNumber(num) {
        return num.toFixed(num > 100 ? 0 : 2);
    }
</script>