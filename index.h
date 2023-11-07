const char index_html[] PROGMEM =
R"==(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>blobFan</title>
  <script src="
https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js
"></script>
  <script src="https://cdn.jsdelivr.net/npm/luxon"></script>
  <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-luxon"></script>
</head>

  <body>
  <div style="width: 800px"><canvas id="chart"></canvas></div>
  <script>
        let data = [ %TEMP_DATA% ];
        let battery = [ %BATT_DATA% ];

    const dataObj = {
      labels: data.map(row => row.time),
      datasets: [
        {
          label: "Temp",
          data: data,
          tension: 0.8,
          yAxisID: 'y',
          parsing: {
            yAxisKey: 'temp',
            xAxisKey: 'time'
          }
        },
        {
          label: "Humidity",
          data: data,
          tension: 0.2,
          yAxisID: 'yh',
          parsing: {
            yAxisKey: 'humid',
            xAxisKey: 'time'
          },
        },
        {
          label: "Battery",
          data: battery,
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
  </script>
</body>

</html>
)==";