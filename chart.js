
let query = (feed) => {
    let url = `https://io.adafruit.com/api/v2/nik_nax/feeds/${feed}/data?include=value,created_at`
    let response = fetch(url, {
        method: "GET",
        mode: "cors",
        headers: {
            "X-AIO-Key": "1ecbdc80f434dd13ddb91eba2f30b8e86f6c437d"
        },
    })
    console.log(response.data)
    return response
}

async function renderChart() {
    let humid = await query("humidity");
    console.log(humid)
    let temp = await query("temperature");
    console.log(temp)
    let batt = await query("battery");
    console.log(batt)

    const dataObj = {
        labels: temp.map(row => row.created_at),
        datasets: [
            {
                label: "Temp",
                data: temp,
                tension: 0.8,
                yAxisID: 'y',
                parsing: {
                    yAxisKey: 'temp',
                    xAxisKey: 'time'
                }
            },
            {
                label: "Humidity",
                data: humid,
                tension: 0.2,
                yAxisID: 'yh',
                parsing: {
                    yAxisKey: 'humid',
                    xAxisKey: 'time'
                },
            },
            {
                label: "Battery",
                data: batt,
                yAxisID: 'yb',
                parsing: {
                    yAxisKey: 'raw',
                    xAxisKey: 'time'
                }
            }
        ],
    };
    const configObj = {
        type: "line",
        data: dataObj,
        options: {
            scales: {
                y: {
                    type: 'linear',
                    position: 'left'
                },
                yh: {
                    type: 'linear',
                    position: 'right',
                    reverse: true
                },
                yb: {
                    type: 'linear',
                    axis: 'y',
                    min: 0,
                    max: 100
                },
                x: {
                    type: 'time'
                },
            }
        }
    };

    new Chart(document.getElementById("chart"), configObj);
}

renderChart();
