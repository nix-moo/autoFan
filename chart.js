
let query = async (feed) => {
    let url = `https://io.adafruit.com/api/v2/nik_nax/feeds/${feed}/data?include=value,created_at`
    let request = {
        method: "GET",
        mode: "cors",
        headers: {
            "X-AIO-Key": "1ecbdc80f434dd13ddb91eba2f30b8e86f6c437d"
        },
    }

    let response = await fetch(url, request).then(response => {
        let rjson = response.json()
        console.log(rjson)
        return rjson
    })

    return response
}

const width = 928;
const height = 500;
const marginTop = 20;
const marginRight = 30;
const marginBottom = 30;
const marginLeft = 40;

let humid = query("humidity");
console.log(humid)
let temp = query("temperature");
console.log(temp)
let batt = query("battery");
console.log(batt)

const yBatt = d3.scaleLinear([0, 100], [height - marginBottom, marginTop]);
const tTemp = d3.scaleLinear(d3.extent(temp, t => t.temp), [height - marginBottom, marginTop]);
const yHumid = d3.scaleLinear(d3.extent(humid, h => h.humid), [height - marginBottom, marginTop]);

const hArea = d3.area()
    .defined(d => !isNaN(d.value))
    .x(d => x(d.created_at))
    .y0(y(0))
    .y1(d => y(d.value));

const ybAxis = d3.axisRight().scale(yBatt);
const ytAxis = d3.axisLeft().scale(yTemp);

const x = d3.scaleUtc()
    .domain(d3.extent(humid, h => h.created_at))
    .range([marginLeft, width - marginRight]);


const svg = d3.create("svg")
    .attr("width", width)
    .attr("height", height)
    .attr("viewBox", [0, 0, width, height])
    .attr("style", "max-width: 100%; height: auto;");

svg.append("path")
    .attr("fill", "lightblue")
    .attr("created_at", hArea(humid) )
svg.append("g")
    .attr("transform", `translate(0,${height - marginBottom})`)
    .call(d3.axisBottom(x).ticks(width / 80).tickSizeOuter(0));

chart.append(svg.node());


