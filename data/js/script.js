/*
        espMCU script.js mk'L' -- Main Application Custom JavaScript Library
                                ***  '' ./js/script.js ''
                                  
*/

// Handle Top Navigation Bar
function handleNavBar() {
  var x = document.getElementById("myTopnav");
  if (x.className === "topnav") {
    x.className += " responsive";
  } 
  else {
    x.className = "topnav";
  }
}

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("wifibars").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/wifibars", true);
    xhttp.send();
  }, 5000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("analogpercent").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/analogpercent", true);
    xhttp.send();
  }, 5000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("wifipercent").innerHTML = this.responseText;
        document.getElementById("tablewifipct").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/wifipercent", true);
    xhttp.send();
  }, 5000 ) ;

      setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("wifisig").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/wifisig", true);
    xhttp.send();
  }, 5000 ) ;

      setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("wifiicon").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/wifiicon", true);
    xhttp.send();
  }, 5000 ) ;


function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  var sliderPct = (sliderValue/10.24);  
sliderPct = sliderPct.toFixed(2);
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  document.getElementById("textSliderPercent").innerHTML = sliderPct;
  document.getElementById("textSliderTable").innerHTML = sliderPct;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}


// ******************** [BEGIN] Handle WebSocket GPIO Outputs ********************
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
function onLoad(event) {
  initWebSocket();
}
function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log('Connection opened');
  websocket.send("states");
}
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
} 
function onMessage(event) {
  var myObj = JSON.parse(event.data);
  console.log(myObj);
  for (i in myObj.gpios){
    var output = myObj.gpios[i].output;
    var state = myObj.gpios[i].state;
    console.log(output);
    console.log(state);
    if (state == "1"){
      document.getElementById(output).checked = true;
      document.getElementById(output+"l").innerHTML = "<div class=\"led led-green\"></div>"
      document.getElementById(output+"s").innerHTML = "ON"
      SnackBar({message: "GPIO"+(output)+"Is ON.", icon: "exclamation", status: "success",})
    }
    else{
      document.getElementById(output).checked = false;
      document.getElementById(output+"l").innerHTML = "<div class=\"led led-red\"></div>";
      document.getElementById(output+"s").innerHTML = "OFF"
      SnackBar({message: "GPIO"+(output)+"Is OFF.", icon: "exclamation", status: "danger",})

    }
  }
  console.log(event.data);
}
// Send Requests to Control GPIOs
function toggleCheckbox (element) {
  console.log(element.id);
  websocket.send(element.id);
  if (element.checked){
    document.getElementById(element.id+"l").innerHTML = "<div class=\"led led-green\"></div>";
    document.getElementById(output+"s").innerHTML = "ON"
    SnackBar({message: "GPIO"+(output)+"Is ON.", icon: "exclamation", status: "success",})
  }
  else {
    document.getElementById(element.id+"l").innerHTML = "<div class=\"led led-red\"></div>";
    document.getElementById(output+"s").innerHTML = "OFF"
    SnackBar({message: "GPIO"+(output)+"Is OFF.", icon: "exclamation", status: "danger",})

  }
}
// ******************** [END] Handle WebSocket GPIO Outputs ********************


// ******************** [BEGIN] Handle WebSocket Analog Input + Charts ********************

// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);
// Create Temperature Chart
var chartT = new Highcharts.Chart({
  chart:{ 
    renderTo:'chart-temperature' 
  },
  series: [
    {
      name: 'Analog Input A0 (8-Bit)'
    }
  ],
  title: { 
    text: undefined
  },
  plotOptions: {
    line: { 
      animation: false,
      dataLabels: { 
        enabled: true 
      }
    }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { 
      text: '8-Bit ADC Value (_____/1024)' 
    }
  },
  credits: { 
    enabled: false 
  }
});
// Create Humidity Chart
var chartH = new Highcharts.Chart({
  chart:{ 
    renderTo:'chart-humidity' 
  },
  series: [{
    name: 'WiFi Signal (Percentage)'
  }],
  title: { 
    text: undefined
  },    
  plotOptions: {
    line: { 
      animation: false,
      dataLabels: { 
        enabled: true 
      }
    },
    series: { 
      color: '#FFFFFF' 
    }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { 
      text: 'WiFi Signal (%)'
    }
  },
  credits: { 
    enabled: false 
  }
});
//Plot temperature in the temperature chart
function plotTemperature(value) {
  var x = (new Date()).getTime()
  var y = Number(value);
  if(chartT.series[0].data.length > 40) {
    chartT.series[0].addPoint([x, y], true, true, true);
  } else {
    chartT.series[0].addPoint([x, y], true, false, true);
  }
}
//Plot humidity in the humidity chart
function plotHumidity(value) {
  var x = (new Date()).getTime()
  var y = Number(value);
  if(chartH.series[0].data.length > 40) {
    chartH.series[0].addPoint([x, y], true, true, true);
  } else {
    chartH.series[0].addPoint([x, y], true, false, true);
  }
}


// Create Temperature Gauge
var gaugeTemp = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 180,
  height: 390,
  units: "Analog Input A0 (0-1024 Bit)",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 1024,
  barBeginCircle: false,
  colorValueBoxRect: "#666666",
  colorValueBoxRectEnd: "#666666",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
      "0",
      "100",
      "200",
      "300",
      "400",
      "500",
      "600",
      "700",
      "800",
      "900",
      "1024"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 0,
          "to": 0,
          "color": "rgba(255, 50, 50, .75)"
      }
  ],
  colorPlate: "#232323",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#000000",
  borderShadowWidth: 0,
  borders: false,
   numberSide: "left",
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 5,
  needleCircleOuter: true,
  needleCircleInner: true,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
}).draw();
  
// Create Humidity Gauge
var gaugeHum = new RadialGauge({
  renderTo: 'gauge-humidity',
width: 220,
    height: 220,
    units: "%",
    title: "WiFi Signal",
    minValue: 0,
    maxValue: 100,
    majorTicks: [
        0,
        10,
        20,
        30,
        40,
        50,
        60,
        70,
        80,
        90,
        100
    ],
    minorTicks: 2,
    strokeTicks: true,
    highlights: [
        {
            "from": 0,
            "to": 30,
            "color": "rgba(210,43,43, .4)"
        },
        {
            "from": 30,
            "to": 60,
            "color": "rgba(255, 255, 0, .4)"
        },
        {
            "from": 60,
            "to": 100,
            "color": "rgba(0, 255, 0, .4)"
        }
    ],
    ticksAngle: 225,
    startAngle: 67.5,
    colorMajorTicks: "#ddd",
    colorMinorTicks: "#ddd",
    colorTitle: "#eee",
    colorUnits: "#ccc",
    colorNumbers: "#eee",
    colorPlate: "#222",
    borderShadowWidth: 0,
    borders: true,
    needleType: "arrow",
    needleWidth: 2,
    needleCircleSize: 7,
    needleCircleOuter: true,
    needleCircleInner: false,
    animationDuration: 1500,
    animationRule: "linear",
    colorBorderOuter: "#333",
    colorBorderOuterEnd: "#111",
    colorBorderMiddle: "#222",
    colorBorderMiddleEnd: "#111",
    colorBorderInner: "#111",
    colorBorderInnerEnd: "#333",
    colorNeedleShadowDown: "#333",
    colorNeedleCircleOuter: "#333",
    colorNeedleCircleOuterEnd: "#111",
    colorNeedleCircleInner: "#111",
    colorNeedleCircleInnerEnd: "#222",
    valueBoxBorderRadius: 0,
    colorValueBoxRect: "#222",
    colorValueBoxRectEnd: "#333"
}).draw();

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var temp = myObj.temperature;
      var hum = myObj.humidity;
      plotTemperature(temp);
      plotHumidity(hum);
      var a0val = myObj.analog0;
      var a0pct = (a0val/10.24);
      a0pct = a0pct.toFixed(2);
      document.getElementById("a0pctValue").innerHTML = a0pct + "&#37;";
      var rssival = myObj.rssi;
      gaugeTemp.value = a0val;
      gaugeHum.value = rssival;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}
if (!!window.EventSource) {
  var source = new EventSource('/events');
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);
  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    plotTemperature(myObj.temperature);
    plotHumidity(myObj.humidity);
    gaugeTemp.value = myObj.analog0;
    gaugeHum.value = myObj.rssi;
  }, false);
}
// ******************** [BEGIN] Handle WebSocket Analog Input + Charts ********************
