static const char index_html[] PROGMEM =
R"==(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>blobFan</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  </head>

  <body style="width: 300px">
    <div><a href="/">Home</a></div>
    <hr />
    <div style="width: 800px"><canvas id="temp"></canvas></div>
    <!-- <script type="module" src="dimensions.js"></script> -->
    <script type="module">
      (async function () {
        let rawdata;
        fetch("http://hostname/files/temp.txt").then(response => {
          console.log(response);
          rawdata = response.text();
        });
        console.log(rawdata);
        let data = rawdata.split("\n");
        console.log(data);

        let tempData = data.map(ln => {
          let split = ln.split(",");
          split.map(l => l.trim());
          let tempObj = {
            time: split[0],
            temp: split[1],
            humd: split[2],
          };
          console.log(tempObj);
          return tempObj;
        });

        new Chart(document.getElementById("temp"), {
          type: "line",
          data: {
            labels: tempData.map(row => row.time),
            datasets: [
              {
                label: "Temp",
                data: tempData.map(row => row.temp),
              },
              {
                label: "Humidity",
                data: tempData.map(row => row.humd),
              },
            ],
          },
        });
      })();
    </script>
  </body>
</html>
)==";