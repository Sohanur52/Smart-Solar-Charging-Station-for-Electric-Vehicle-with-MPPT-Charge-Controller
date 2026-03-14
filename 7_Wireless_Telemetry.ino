#include <WebServer.h>
#include <WiFi.h>
#include <TimeLib.h> 

// External variables from the main tab
// External variables from the main tab
extern WebServer server;
extern int temperature, batteryPercent, dem, pinValue2, PWM, pwmMax; 
extern float voltageInput, voltageOutput, currentInput, currentOutput;
extern float powerInput, Wh, electricalPrice, voltageBatteryMax, voltageBatteryMin;
extern unsigned long secondsElapsed;
extern bool MPPT_Mode, output_Mode;
extern int temperatureFan;
extern float currentCharging;
extern float powerInputAdjusted;    // ADD THIS
extern float currentInputAdjusted;  // ADD THIS
#define Load 12 

// Using R"rawliteral(...)rawliteral" fixes the "Data not updating" issue
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>MPPT PRO Dashboard</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
<style>
body{font-family:Arial,sans-serif;background:#121212;color:#e0e0e0;margin:0;padding:20px;}
.header{padding:10px;border-bottom:2px solid #00e676;margin-bottom:20px;text-align:center;position:relative;}
.datetime-container{position:absolute;top:10px;right:20px;text-align:right;}
.datetime{font-size:16px;color:#00e676;font-weight:bold;}
.date{font-size:14px;color:#888;margin-top:5px;}
.location{font-size:12px;color:#666;margin-top:3px;font-style:italic;}

/* Control Panel Styles */
.control-panel{background:#1e1e1e;border-radius:12px;padding:20px;margin-bottom:20px;border:1px solid #333;}
.control-panel h3{margin-top:0;color:#00e676;border-bottom:1px solid #333;padding-bottom:10px;}
.control-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px;}
.control-item{background:#2a2a2a;padding:15px;border-radius:8px;border:1px solid #333;}
.control-label{font-size:14px;color:#888;margin-bottom:8px;display:block;}
.control-value{font-size:18px;color:#fff;margin-bottom:10px;display:block;}
.control-buttons{display:flex;gap:5px;flex-wrap:wrap;}
.control-btn{background:#333;border:1px solid #444;color:white;padding:8px 12px;border-radius:5px;cursor:pointer;font-size:13px;transition:all 0.3s;}
.control-btn.active{background:#00e676;color:#121212;border-color:#00e676;}
.control-btn:hover{background:#444;}
.control-btn.danger{background:#ff5252;color:white;}
.control-btn.danger:hover{background:#ff7070;}
.control-slider{width:100%;margin:10px 0;}
.slider-value{color:#00e676;font-weight:bold;font-size:16px;}
.control-input{background:#333;border:1px solid #444;color:white;padding:8px;border-radius:5px;width:80px;margin-right:5px;}

.container{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:15px;margin-bottom:30px;}
.card{background:#1e1e1e;padding:20px;border-radius:12px;border:1px solid #333;text-align:center;}
.val{display:block;font-size:24px;font-weight:bold;color:#00e676;margin-top:10px;}
.unit{font-size:14px;color:#888;}
button{padding:12px;border:none;border-radius:8px;font-weight:bold;cursor:pointer;margin:5px;width:45%;}
.btn-on{background:#00e676;color:#121212;}
.btn-off{background:#ff5252;color:white;}
input{background:#333;border:1px solid #444;color:white;padding:8px;border-radius:5px;width:60px;}
.chart-container{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:15px;margin-top:20px;}
.chart-card{background:#1e1e1e;padding:15px;border-radius:12px;border:1px solid #333;height:300px;}
.full-width{grid-column:1/-1;height:350px;}

/* Historical chart styles */
.historical-container {
  margin-top: 30px;
  background: #1e1e1e;
  padding: 20px;
  border-radius: 12px;
  border: 1px solid #333;
}

.tab-container {
  display: flex;
  margin-bottom: 15px;
  border-bottom: 1px solid #333;
}

.tab {
  padding: 10px 20px;
  cursor: pointer;
  background: #2a2a2a;
  margin-right: 5px;
  border-radius: 5px 5px 0 0;
  border: 1px solid #333;
  border-bottom: none;
}

.tab.active {
  background: #00e676;
  color: #121212;
  font-weight: bold;
}

.historical-chart {
  height: 400px;
  margin-top: 20px;
  width: 100%;
}

.controls-row {
  display: flex;
  flex-wrap: wrap;
  gap: 15px;
  margin-bottom: 20px;
  align-items: center;
}

.control-group {
  display: flex;
  flex-direction: column;
  min-width: 150px;
}

.control-group label {
  margin-bottom: 5px;
  color: #ccc;
  font-size: 14px;
}

.control-group select {
  background: #333;
  border: 1px solid #444;
  color: white;
  padding: 8px;
  border-radius: 5px;
  font-size: 14px;
}

.action-buttons {
  display: flex;
  gap: 10px;
  margin-top: 20px;
  justify-content: center;
}

.action-button {
  padding: 10px 20px;
  border: none;
  border-radius: 5px;
  cursor: pointer;
  font-weight: bold;
} 

.btn-refresh {
  background: #2196F3;
  color: white;
}

.btn-test {
  background: #4CAF50;
  color: white;
}
</style></head><body>
<div class='header'>
  <h1>SMART MPPT SOLAR SYSTEM</h1>
  <div class='datetime-container'>
    <div class='datetime' id='currentTime'>12:00:00 PM</div>
    <div class='date' id='currentDate'>Wednesday, February 10, 2026</div>
    <div class='location' id='location'>Dhaka, Bangladesh</div>
  </div>
</div>

<!-- Control Panel Section -->
<div class='control-panel'>
  <h3> System Controls</h3>
  <div class='control-grid'>
    <!-- Algorithm Control -->
    <div class='control-item'>
      <span class='control-label'>Charging Algorithm</span>
      <span class='control-value' id='currentAlgorithm'>Loading...</span>
      <div class='control-buttons'>
        <button class='control-btn' id='algoMPPT' onclick='setAlgorithm(1)'>MPPT+CC-CV</button>
        <button class='control-btn' id='algoCCCV' onclick='setAlgorithm(0)'>CC-CV Only</button>
      </div>
    </div>

    <!-- Mode Control -->
    <div class='control-item'>
      <span class='control-label'>Operating Mode</span>
      <span class='control-value' id='currentMode'>Loading...</span>
      <div class='control-buttons'>
        <button class='control-btn' id='modeCharger' onclick='setMode(1)'>Charger</button>
        <button class='control-btn' id='modePSU' onclick='setMode(0)'>PSU</button>
      </div>
    </div>

    <!-- Max Voltage Control -->
    <div class='control-item'>
      <span class='control-label'>Max Battery Voltage</span>
      <span class='control-value' id='maxVoltageDisplay'>Loading...</span>
      <div class='control-buttons'>
        <input type='number' id='maxVoltageInput' class='control-input' step='0.01' min='10' max='50' placeholder='27.30'>
        <button class='control-btn' onclick='setMaxVoltage()'>Set</button>
      </div>
    </div>

    <!-- Min Voltage Control -->
    <div class='control-item'>
      <span class='control-label'>Min Battery Voltage</span>
      <span class='control-value' id='minVoltageDisplay'>Loading...</span>
      <div class='control-buttons'>
        <input type='number' id='minVoltageInput' class='control-input' step='0.01' min='5' max='30' placeholder='10.40'>
        <button class='control-btn' onclick='setMinVoltage()'>Set</button>
      </div>
    </div>

    <!-- Max Current Control -->
    <div class='control-item'>
  <span class='control-label'>Max Charging Current</span>
  <span class='control-value' id='maxCurrentDisplay'>Loading...</span>
  <div class='control-buttons'>
    <input type='number' id='maxCurrentInput' class='control-input' step='0.01' min='0' max='50' placeholder='30.00'>
    <button class='control-btn' onclick='setMaxCurrent()'>Set</button>
  </div>
    </div>

    <!-- Fan Temperature Control -->
    <div class='control-item'>
      <span class='control-label'>Fan Trigger Temp</span>
      <span class='control-value' id='fanTempDisplay'>Loading...</span>
      <div class='control-buttons'>
        <input type='range' id='fanTempSlider' class='control-slider' min='30' max='90' value='60' oninput='updateFanTempDisplay(this.value)'>
        <span class='slider-value' id='fanTempSliderValue'>60 °C</span>
        <button class='control-btn' onclick='setFanTemp()'>Set</button>
      </div>
    </div>

    <!-- Load Control -->
    <div class='control-item'>
      <span class='control-label'>Load Control</span>
      <span class='control-value' id='currentLoadState'>Loading...</span>
      <div class='control-buttons'>
        <button class='control-btn' id='loadWebOn' onclick='sendLoadCommand(1)'>LOAD ON</button>
        <button class='control-btn' id='loadWebOff' onclick='sendLoadCommand(0)'>LOAD OFF</button>
      </div>
    </div>

    <!-- Factory Reset -->
    <div class='control-item'>
      <span class='control-label'>Factory Reset</span>
      <span class='control-value' style='color:#ff5252;'> Warning</span>
      <div class='control-buttons'>
        <button class='control-btn danger' onclick='confirmFactoryReset()'>Reset to Defaults</button>
      </div>
    </div>
  </div>
</div>

<div class='container'>
  <div class='card'>Solar Voltage<span class='val' id='vi'>0.0</span><span class='unit'>V</span></div>
  <div class='card'>Solar Current<span class='val' id='ii'>0.0</span><span class='unit'>A</span></div>
  <div class='card'>Solar Power<span class='val' id='pi'>0.0</span><span class='unit'>W</span></div>
  <div class='card'>SOC<span class='val' id='soc'>0</span><span class='unit'>%</span></div>
</div>
<div class='container'>
  <div class='card'>Battery Voltage<span class='val' id='vo'>0.0</span><span class='unit'>V</span></div>
  <div class='card'>Battery Current<span class='val' id='io'>0.0</span><span class='unit'>A</span></div>
  <div class='card'>Output Power<span class='val' id='po'>0.0</span><span class='unit'>W</span></div>
  <div class='card'>Temperature<span class='val' id='temp'>0.0</span><span class='unit'>C</span></div>
</div>
<div class='container'>
  <div class='card'>Efficiency<span class='val' id='eff'>0.0</span><span class='unit'>%</span></div>
  <div class='card'>PWM Duty<span class='val' id='pwm_val'>0</span><span class='unit'>%</span></div>
  <div class='card'>Energy Used<span class='val' id='wh'>0.0</span><span class='unit'>Wh</span></div>
  <div class='card'>Saved cost<span class='val' id='Saved cost'>0.0</span><span class='unit'>BDT</span></div>
</div>
<div class='chart-container'>
  <div class='chart-card'><canvas id='voltageChart'></canvas></div>
  <div class='chart-card'><canvas id='currentChart'></canvas></div>
</div>
<div class='chart-container'>
  <div class='chart-card full-width'><canvas id='powerChart'></canvas></div>
</div>

<!-- Historical Data Section -->
<div class='historical-container'>
  <h3> Historical Data Analysis</h3>
  
  <div class='controls-row'>
    <div class='control-group'>
      <label>Time Duration:</label>
      <select id='timeDuration' onchange='updateHistoricalChart()'>
        <option value='12'>Last 12 Hours</option>
        <option value='24' selected>Last 24 Hours (1 Day)</option>
        <option value='48'>Last 48 Hours (2 Days)</option>
        <option value='72'>Last 72 Hours (3 Days)</option>
        <option value='96'>Last 96 Hours (4 Days)</option>
        <option value='120'>Last 120 Hours (5 Days)</option>
      </select>
    </div>
    
    <div class='control-group'>
      <label>Chart Type:</label>
      <select id='chartType' onchange='changeChartType()'>
        <option value='voltage'>Voltage</option>
        <option value='current'>Current</option>
        <option value='power'>Power</option>
        <option value='temperature'>Temperature</option>
        <option value='efficiency'>Efficiency</option>
      </select>
    </div>
    
    <div class='control-group'>
      <label>Data Resolution:</label>
      <select id='dataResolution' onchange='updateHistoricalChart()'>
        <option value='realtime'>Real-time (1 min)</option>
        <option value='30min' selected>30 Minutes</option>
        <option value='1hour'>1 Hour</option>
        <option value='2hour'>2 Hours</option>
        <option value='6hour'>6 Hours</option>
      </select>
    </div>
  </div>
  
  <div class='tab-container'>
    <button class='tab active' onclick='showTabChart("voltage")'>Voltage</button>
    <button class='tab' onclick='showTabChart("current")'>Current</button>
    <button class='tab' onclick='showTabChart("power")'>Power</button>
    <button class='tab' onclick='showTabChart("temperature")'>Temperature</button>
    <button class='tab' onclick='showTabChart("efficiency")'>Efficiency</button>
  </div>
  
  <div class='historical-chart'>
    <canvas id='historicalChart'></canvas>
  </div>
  
  <div class='action-buttons'>
    <button class='action-button btn-refresh' onclick='loadHistoricalData()'> Refresh Data</button>
    <!-- <button class='action-button btn-test' onclick='testChart()'>🧪 Test Chart</button> -->
  </div>
  
  <div id='chartInfo' style='margin-top: 15px; color: #888; font-size: 12px; text-align: center;'>
    Data points: <span id='pointCount'>0</span> | Resolution: <span id='resolution'>--</span> | Last update: <span id='lastUpdate'>--</span>
  </div>
</div>

<div class='card' style='margin-top:10px;'>
  Price: <input type='number' id='p' step='0.1' value='6.0'> <button onclick='setPrice()'>Save</button>
  <br><br>
  <button style='width:95%;background:#2196F3;color:white;' onclick='syncTime()'>SYNC SYSTEM TIME</button>
  <br><br>
  <button style='width:95%;background:#4CAF50;color:white;' onclick='showRemoteAccess()'> SETUP REMOTE ACCESS</button>
</div>

<script>

window.electricalPrice = 10.0;
let lastPrice = 6.0;

// Chart instances
let voltageChart, currentChart, powerChart, historicalChart = null;
let currentChartType = 'voltage';
let currentDuration = 24;
let currentResolution = '30min';
let historicalData = [];
let lastDataUpdate = 0;
let chartUpdateInterval = 60000; // Update charts every 1 minute (60000 ms)

function testChart() {
  alert('Test chart - use Refresh Data to load actual data');
  loadHistoricalData();
}

// Update historical chart based on selected parameters
function updateHistoricalChart() {
  console.log("Dropdown changed, reloading historical data...");
  loadHistoricalData();
}

// Initialize real-time charts with Y-axis starting from 0
function initCharts() {
  const vCtx = document.getElementById('voltageChart').getContext('2d');
  const cCtx = document.getElementById('currentChart').getContext('2d');
  const pCtx = document.getElementById('powerChart').getContext('2d');

  voltageChart = new Chart(vCtx, { 
    type: 'line', 
    data: { 
      labels: [], 
      datasets: [
        { label: 'Solar V', borderColor: '#00e676', data: [], fill: false, tension: 0.4 },
        { label: 'Battery V', borderColor: '#2196F3', data: [], fill: false, tension: 0.4 }
      ] 
    }, 
    options: { 
      responsive: true, 
      maintainAspectRatio: false,
      plugins: {
        legend: { labels: { color: '#ccc' } }
      },
      scales: {
        x: { 
          ticks: { color: '#ccc', maxTicksLimit: 10 }, // Feature 3: Show about 10-12 labels
          grid: { color: 'rgba(255,255,255,0.1)' }
        },
        y: { 
          // Feature 2: Start from 0
          min: 0,
          beginAtZero: true,
          ticks: { color: '#ccc' }, 
          grid: { color: 'rgba(255,255,255,0.1)' } 
        }
      }
    }
  });
  
  currentChart = new Chart(cCtx, { 
    type: 'line', 
    data: { 
      labels: [], 
      datasets: [
        { label: 'Solar I', borderColor: '#FF9800', data: [], fill: false, tension: 0.4 },
        { label: 'Battery I', borderColor: '#E91E63', data: [], fill: false, tension: 0.4 }
      ] 
    }, 
    options: { 
      responsive: true, 
      maintainAspectRatio: false,
      plugins: {
        legend: { labels: { color: '#ccc' } }
      },
      scales: {
        x: { 
          ticks: { color: '#ccc', maxTicksLimit: 10 }, // Feature 3: Show about 10-12 labels
          grid: { color: 'rgba(255,255,255,0.1)' }
        },
        y: { 
          // Feature 2: Start from 0
          min: 0,
          beginAtZero: true,
          ticks: { color: '#ccc' }, 
          grid: { color: 'rgba(255,255,255,0.1)' } 
        }
      }
    }
  });
  
  powerChart = new Chart(pCtx, { 
    type: 'line', 
    data: { 
      labels: [], 
      datasets: [
        { label: 'Solar W', borderColor: '#4CAF50', data: [], fill: false, tension: 0.4 },
        { label: 'Efficiency', borderColor: '#FFC107', data: [], yAxisID: 'y1', fill: false, tension: 0.4 }
      ] 
    }, 
    options: { 
      responsive: true, 
      maintainAspectRatio: false,
      plugins: {
        legend: { labels: { color: '#ccc' } }
      },
      scales: {
        x: { 
          ticks: { color: '#ccc', maxTicksLimit: 10 }, // Feature 3: Show about 10-12 labels
          grid: { color: 'rgba(255,255,255,0.1)' } 
        },
        y: { 
          // Feature 2: Start from 0
          min: 0,
          beginAtZero: true,
          type: 'linear',
          position: 'left',
          ticks: { color: '#ccc' }, 
          grid: { color: 'rgba(255,255,255,0.1)' } 
        },
        y1: { 
          position: 'right', 
          min: 0, 
          max: 100, 
          beginAtZero: true,
          ticks: { color: '#ccc' }, 
          grid: { drawOnChartArea: false } 
        }
      }
    }
  });
}

// Update datetime display
function updateDateTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString('en-US', { 
    hour12: true,
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  });
  const dateString = now.toLocaleDateString('en-US', {
    weekday: 'long',
    year: 'numeric',
    month: 'long',
    day: 'numeric'
  });
  document.getElementById('currentTime').textContent = timeString;
  document.getElementById('currentDate').textContent = dateString;
  document.getElementById('lastUpdate').textContent = timeString;
}

// Control functions
function setMaxCurrent() {
  let value = document.getElementById('maxCurrentInput').value;
  if (value) {
    document.getElementById('maxCurrentDisplay').textContent = parseFloat(value).toFixed(2) + ' A';
    
    fetch('/setMaxCurrent?value=' + value)
      .then(response => response.text())
      .then(data => {
        console.log("Successfully set to: " + value);
      })
      .catch(error => {
        console.log('Error:', error);
        setTimeout(updateData, 1000);
      });
  }
}

function setAlgorithm(mode) {
  fetch('/setAlgorithm?mode=' + mode)
    .then(response => response.text())
    .then(data => {
      setTimeout(updateData, 500);
    });
}

function setMode(mode) {
  fetch('/setMode?mode=' + mode)
    .then(response => response.text())
    .then(data => {
      setTimeout(updateData, 500);
    });
}

function setMaxVoltage() {
  let value = document.getElementById('maxVoltageInput').value;
  if (value) {
    fetch('/setMaxVoltage?value=' + value)
      .then(response => response.text())
      .then(data => {
        setTimeout(updateData, 500);
      });
  }
}

function setMinVoltage() {
  let value = document.getElementById('minVoltageInput').value;
  if (value) {
    fetch('/setMinVoltage?value=' + value)
      .then(response => response.text())
      .then(data => {
        setTimeout(updateData, 500);
      });
  }
}

function updateFanTempDisplay(value) {
  document.getElementById('fanTempSliderValue').textContent = value + '°C';
}

function setFanTemp() {
  let value = document.getElementById('fanTempSlider').value;
  fetch('/setFanTemp?temp=' + value)
    .then(response => response.text())
    .then(data => {
      setTimeout(updateData, 500);
    });
}

function sendLoadCommand(state) {
  fetch('/control?load=' + state)
    .then(response => response.text())
    .then(data => {
      setTimeout(updateData, 500);
    });
}

function setPrice() { 
  let price = parseFloat(document.getElementById('p').value);
  if (!isNaN(price)) {
    window.electricalPrice = price;
    lastPrice = price;
    fetch('/setPrice?price=' + price)
      .then(response => response.text())
      .then(data => {
        let wh = parseFloat(document.getElementById('wh').innerText);
        document.getElementById('Saved cost').innerText = ((wh / 1000) * price).toFixed(2);
      });
  }
}

function confirmFactoryReset() {
  if (confirm(' WARNING: This will reset all settings to factory defaults! Are you sure?')) {
    fetch('/factoryReset')
      .then(response => response.text())
      .then(data => {
        alert('Factory reset completed! System will reload settings.');
        setTimeout(() => location.reload(), 2000);
      });
  }
}

// Feature 5: Setup Remote Access function with both IPs
function showRemoteAccess() {
  // Fetch network info first
  fetch('/network')
    .then(res => res.json())
    .then(data => {
      let message = '🔌 REMOTE ACCESS SETUP\n';
      message += '═══════════════════════\n\n';
      
      message += '📡 NETWORK INFORMATION:\n';
      message += '────────────────────\n';
      message += '📶 WiFi Mode: ';
      if (data.wifi_mode == 1) message += 'Station Only\n';
      else if (data.wifi_mode == 2) message += 'Access Point Only\n';
      else if (data.wifi_mode == 3) message += 'Access Point + Station\n';
      
      message += '🟢 Router IP: ' + data.wifi_ip + '\n';
      message += '🟡 Hotspot IP: ' + data.hotspot_ip + '\n';
      message += '📛 SSID: ' + data.wifi_ssid + '\n';
      message += '📶 Signal: ' + data.rssi + ' dBm\n\n';
      
      message += '📱 LOCAL ACCESS:\n';
      message += '────────────────────\n';
      message += '• Via Router: http://' + data.wifi_ip + '\n';
      message += '• Via Hotspot: http://' + data.hotspot_ip + '\n\n';
      
      message += '🌍 REMOTE ACCESS OPTIONS:\n';
      message += '────────────────────\n';
      message += 'Option 1: Use ngrok (Free)\n';
      message += '1. Install ngrok from ngrok.com\n';
      message += '2. Run: ngrok http 80\n';
      message += '3. Share the ngrok URL\n\n';
      
      message += 'Option 2: Port Forwarding\n';
      message += '1. Forward port 80 on your router\n';
      message += '2. Access via your public IP\n\n';
      
      message += 'Option 3: Use Hotspot (No Internet)\n';
      message += '1. Connect to WiFi: ESP32_MPPT_PRO\n';
      message += '2. Password: 12345678\n';
      message += '3. Open: http://' + data.hotspot_ip + '\n';
      
      alert(message);
    })
    .catch(error => {
      // Fallback if network fetch fails
      let message = 'Remote Access Options:\n\n';
      message += 'Local IP: ' + window.location.hostname + '\n';
      message += 'Hotspot IP: 192.168.4.1\n\n';
      message += 'Port forwarding: Forward port 80';
      alert(message);
    });
  
  // Open ngrok website in new tab
  
}

// Load historical data from server
function loadHistoricalData() {
  console.log("Loading historical data...");
  
  fetch('/historical')
    .then(res => {
      console.log("Response status:", res.status);
      if (!res.ok) {
        throw new Error('Server returned error: ' + res.status);
      }
      return res.json();
    })
    .then(data => {
      console.log("Received historical data:", data);
      historicalData = data;
      
      // Check if we have any data
      let hasData = false;
      if (historicalData && historicalData.length > 0) {
        for (let day = 0; day < historicalData.length; day++) {
          if (historicalData[day].data && historicalData[day].data.length > 0) {
            hasData = true;
            break;
          }
        }
      }
      
      if (!hasData) {
        console.log("No historical data received");
        showEmptyHistoricalChart();
      } else {
        updateHistoricalChartWithRealData();
      }
    })
    .catch(error => {
      console.log('Error loading historical data:', error);
      showEmptyHistoricalChart();
    });
}

// Show empty chart with message
function showEmptyHistoricalChart() {
  const ctx = document.getElementById('historicalChart').getContext('2d');
  
  if (historicalChart) {
    historicalChart.destroy();
  }
  
  historicalChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: ['No Data'],
      datasets: [{
        label: 'No historical data available',
        data: [0],
        borderColor: '#888',
        backgroundColor: 'rgba(136, 136, 136, 0.1)'
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { labels: { color: '#ccc' } },
        title: {
          display: true,
          text: 'No historical data - System will collect data every minute',
          color: '#ff5252',
          font: { size: 14 }
        }
      },
      scales: {
        y: {
          min: 0,
          beginAtZero: true
        }
      }
    }
  });
  
  document.getElementById('pointCount').textContent = '0';
  document.getElementById('resolution').textContent = '--';
}

// Update historical chart with real data
function updateHistoricalChartWithRealData() {
  if (!historicalData || historicalData.length === 0) {
    showEmptyHistoricalChart();
    return;
  }
  
  const ctx = document.getElementById('historicalChart').getContext('2d');
  
  if (historicalChart) {
    historicalChart.destroy();
  }
  
  // Collect all data points
  let allData = [];
  let allLabels = [];
  
  // Get current chart type
  currentChartType = document.getElementById('chartType').value;
  
  // Process each day
  for (let day = 0; day < historicalData.length; day++) {
    const dayData = historicalData[day];
    if (!dayData || !dayData.data) continue;
    
    // Process each hour in this day
    for (let hour = 0; hour < dayData.data.length; hour++) {
      const point = dayData.data[hour];
      if (!point) continue;
      
      // Create label with hour and date
      const label = point.time + ' ' + dayData.date.substring(5); // Show time and month-day
      allData.push(point);
      allLabels.push(label);
    }
  }
  
  if (allData.length === 0) {
    showEmptyHistoricalChart();
    return;
  }
  
  // Sort by time (assuming data is in order)
  let labels = allLabels;
  
  let datasets = [];
  let yAxisLabel = '';
  let chartTitle = '';
  
  switch(currentChartType) {
    case 'voltage':
      datasets = [
        { 
          label: 'Solar Voltage (V)', 
          borderColor: '#00e676', 
          data: allData.map(d => d.vi || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(0, 230, 118, 0.1)'
        },
        { 
          label: 'Battery Voltage (V)', 
          borderColor: '#2196F3', 
          data: allData.map(d => d.vo || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(33, 150, 243, 0.1)'
        }
      ];
      yAxisLabel = 'Voltage (V)';
      chartTitle = 'Historical Voltage Data';
      break;
    case 'current':
      datasets = [
        { 
          label: 'Solar Current (A)', 
          borderColor: '#FF9800', 
          data: allData.map(d => d.ii || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(255, 152, 0, 0.1)'
        },
        { 
          label: 'Battery Current (A)', 
          borderColor: '#E91E63', 
          data: allData.map(d => d.io || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(233, 30, 99, 0.1)'
        }
      ];
      yAxisLabel = 'Current (A)';
      chartTitle = 'Historical Current Data';
      break;
    case 'power':
      datasets = [
        { 
          label: 'Solar Power (W)', 
          borderColor: '#4CAF50', 
          data: allData.map(d => d.pi || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(76, 175, 80, 0.1)'
        }
      ];
      yAxisLabel = 'Power (W)';
      chartTitle = 'Historical Power Data';
      break;
    case 'temperature':
      datasets = [
        { 
          label: 'Temperature (°C)', 
          borderColor: '#FF5722', 
          data: allData.map(d => d.temp || 0), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(255, 87, 34, 0.1)'
        }
      ];
      yAxisLabel = 'Temperature (°C)';
      chartTitle = 'Historical Temperature Data';
      break;
    case 'efficiency':
      datasets = [
        { 
          label: 'Efficiency (%)', 
          borderColor: '#FFC107', 
          data: allData.map(d => {
            if (d.pi && d.pi > 0) {
              return ((d.vo * d.io) / d.pi * 100).toFixed(1);
            }
            return 0;
          }), 
          fill: false, 
          tension: 0.4,
          backgroundColor: 'rgba(255, 193, 7, 0.1)'
        }
      ];
      yAxisLabel = 'Efficiency (%)';
      chartTitle = 'Historical Efficiency Data';
      break;
  }
  
  historicalChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: labels,
      datasets: datasets
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          labels: { color: '#ccc' }
        },
        title: {
          display: true,
          text: chartTitle + ' - ' + allData.length + ' data points',
          color: '#ccc',
          font: { size: 16 }
        }
      },
      scales: {
        x: {
          title: { display: true, text: 'Time', color: '#ccc' },
          ticks: { 
            color: '#ccc', 
            maxTicksLimit: 12,
            callback: function(value, index, values) {
              const skip = Math.ceil(labels.length / 12);
              return index % skip === 0 ? labels[index] : '';
            }
          },
          grid: { color: 'rgba(255,255,255,0.1)' }
        },
        y: {
          title: { display: true, text: yAxisLabel, color: '#ccc' },
          // Feature 2: Start from 0
          min: 0,
          beginAtZero: true,
          ticks: { color: '#ccc' },
          grid: { color: 'rgba(255,255,255,0.1)' }
        }
      }
    }
  });
  
  document.getElementById('pointCount').textContent = allData.length;
  document.getElementById('resolution').textContent = '1 minute';
}

// Update real-time data
function updateData() {
  fetch('/data').then(res => res.json()).then(data => {
    // Feature 1: Check for "No Battery" or "No Power Source" strings
    let viDisplay = data.vi;
    let voDisplay = data.vo;
    let iiDisplay = data.ii;
    let piDisplay = data.pi;
    
    // Handle string values for display
    if (typeof data.vi === 'string') {
      viDisplay = data.vi;
      iiDisplay = "0.00";
      piDisplay = "0.0";
    }
    if (typeof data.vo === 'string') {
      voDisplay = data.vo;
    }
    
    // Update display values
    document.getElementById('vi').innerText = viDisplay;
    document.getElementById('ii').innerText = iiDisplay;
    document.getElementById('pi').innerText = piDisplay;
    document.getElementById('soc').innerText = data.soc;
    document.getElementById('vo').innerText = voDisplay;
    document.getElementById('io').innerText = data.io.toFixed(2);
    document.getElementById('po').innerText = (typeof data.vo === 'number' ? data.vo * data.io : 0).toFixed(1);
    document.getElementById('temp').innerText = data.temp.toFixed(1);
    document.getElementById('eff').innerText = data.eff.toFixed(1);
    document.getElementById('pwm_val').innerText = data.pwm;
    document.getElementById('wh').innerText = data.wh.toFixed(1);
    document.getElementById('Saved cost').innerText = ((data.wh / 1000) * window.electricalPrice).toFixed(2);
    
    // Update control panel display values
    document.getElementById('currentAlgorithm').textContent = data.algo == 1 ? 'MPPT + CC-CV' : 'CC-CV Only';
    document.getElementById('currentMode').textContent = data.mode == 1 ? 'Charger Mode' : 'PSU Mode';
    document.getElementById('maxVoltageDisplay').textContent = data.maxV.toFixed(2) + ' V';
    document.getElementById('minVoltageDisplay').textContent = data.minV.toFixed(2) + ' V';
    
    if (data.maxCurrent && !isNaN(data.maxCurrent)) {
      document.getElementById('maxCurrentDisplay').textContent = data.maxCurrent.toFixed(2) + ' A';
    }
    
    document.getElementById('fanTempDisplay').textContent = data.fanTemp + ' °C';
    document.getElementById('fanTempSlider').value = data.fanTemp;
    document.getElementById('fanTempSliderValue').textContent = data.fanTemp + '°C';
    
    // Update active states
    document.getElementById('algoMPPT').classList.toggle('active', data.algo == 1);
    document.getElementById('algoCCCV').classList.toggle('active', data.algo == 0);
    document.getElementById('modeCharger').classList.toggle('active', data.mode == 1);
    document.getElementById('modePSU').classList.toggle('active', data.mode == 0);
    
    // Update load state
    document.getElementById('currentLoadState').textContent = data.load === 1 ? 'ON' : 'OFF';
    document.getElementById('loadWebOn').classList.toggle('active', data.load === 1);
    document.getElementById('loadWebOff').classList.toggle('active', data.load === 0);
    
    // Update real-time charts with 1-minute intervals
    const now = new Date().toLocaleTimeString();
    
    [voltageChart, currentChart, powerChart].forEach(c => { 
      if(c.data.labels.length > 30) { 
        c.data.labels.shift(); 
        c.data.datasets.forEach(d => d.data.shift()); 
      } 
      c.data.labels.push(now); 
    });
    
    // Use numeric values for charts (0 if string)
    voltageChart.data.datasets[0].data.push(typeof data.vi === 'number' ? data.vi : 0);
    voltageChart.data.datasets[1].data.push(typeof data.vo === 'number' ? data.vo : 0);
    currentChart.data.datasets[0].data.push(typeof data.ii === 'number' ? data.ii : 0);
    currentChart.data.datasets[1].data.push(data.io);
    powerChart.data.datasets[0].data.push(typeof data.pi === 'number' ? data.pi : 0);
    powerChart.data.datasets[1].data.push(data.eff);
    
    [voltageChart, currentChart, powerChart].forEach(c => c.update('none'));
    
    // Feature 4: Auto-refresh historical data when new point is received
    // Check if it's time to refresh (every minute)
    const nowTime = Date.now();
    if (nowTime - lastDataUpdate >= chartUpdateInterval) {
      lastDataUpdate = nowTime;
      console.log("Auto-refreshing historical data (1-minute interval)");
      loadHistoricalData();
    }
    
  }).catch(error => {
    console.log('Error fetching data:', error);
  });
}

// Change chart type from dropdown
function changeChartType() {
  currentChartType = document.getElementById('chartType').value;
  
  // Update tabs
  document.querySelectorAll('.tab').forEach(tab => {
    tab.classList.remove('active');
  });
  const tabs = document.querySelectorAll('.tab');
  const tabIndex = ['voltage', 'current', 'power', 'temperature', 'efficiency'].indexOf(currentChartType);
  if (tabIndex >= 0 && tabs[tabIndex]) {
    tabs[tabIndex].classList.add('active');
  }
  
  // Update chart with real data
  if (historicalData && historicalData.length > 0) {
    updateHistoricalChartWithRealData();
  } else {
    loadHistoricalData();
  }
}

// Show chart from tabs
function showTabChart(type) {
  currentChartType = type;
  document.getElementById('chartType').value = type;
  changeChartType();
}

// Initialize on page load
window.addEventListener('load', () => {
  console.log("Page loaded, initializing...");
  
  initCharts();
  updateDateTime();
  
  // Load saved price from server
  fetch('/getPrice').then(res => res.text()).then(price => {
    if (price) {
      window.electricalPrice = parseFloat(price);
      lastPrice = parseFloat(price);
      document.getElementById('p').value = price;
    }
  }).catch(() => {});
  
  setInterval(updateDateTime, 1000);
  setInterval(updateData, 2000);
  
  setTimeout(() => {
    console.log("Loading historical data...");
    loadHistoricalData();
    lastDataUpdate = Date.now();
  }, 1000);
});
</script></body></html>
)=====";

// ============ HANDLER FUNCTIONS ============

// External variables
extern float Wh;
extern float electricalPrice;

// EEPROM addresses
#define EE_WH_ADDR 200
#define EE_PRICE_ADDR 204

void handleRoot() { 
  server.send(200, "text/html", INDEX_HTML);
}

void handleData() {
  float efficiency = 0.0;
  
  // Define the adjusted values HERE (they come from sensors but need to be accessible)
  float esp32Power = 3.5;  // ESP32 idle power consumption
  
  float powerInputAdjusted = powerInput + esp32Power;
  float currentInputAdjusted = (voltageInput > 0) ? powerInputAdjusted / voltageInput : 0;
  
  // Use ADJUSTED values for efficiency calculation
  if (powerInputAdjusted > 0 && powerOutput > 0) {
    efficiency = (powerOutput / powerInputAdjusted) * 100.0;
    efficiency = constrain(efficiency, 0, 100);
  } else {
    efficiency = 0.0;
  }
  
  float pwm_percentage = 0;
  if (pwmMax > 0) {
    pwm_percentage = (PWM * 100.0) / pwmMax;
  }
  
  int loadState = digitalRead(Load);
  
  // Check for error conditions
  String voltageInputDisplay = String(voltageInput, 1);
  String currentInputDisplay = String(currentInputAdjusted, 2);
  String powerInputDisplay = String(powerInputAdjusted, 1);
  String voltageOutputDisplay = String(voltageOutput, 2);
  
  // Feature 1: Show "No Battery" if BNC error
  if (BNC == 1) {
    voltageOutputDisplay = "\"No Battery\"";
  }
  
  // Feature 1: Show "No Power Source" if FLV error or no input voltage
  if (FLV == 1 || voltageInput < 1.0) {
    voltageInputDisplay = "\"No Power Source\"";
    currentInputDisplay = "0.00";
    powerInputDisplay = "0.0";
  }
  
  // Send values to webpage
  String json = "{\"vi\":" + voltageInputDisplay + 
              ",\"ii\":" + currentInputDisplay + 
              ",\"pi\":" + powerInputDisplay + 
              ",\"vo\":" + voltageOutputDisplay + 
              ",\"io\":" + String(currentOutput, 2) + 
              ",\"soc\":" + String(batteryPercent) + 
              ",\"temp\":" + String((float)temperature, 1) + 
              ",\"eff\":" + String(efficiency, 1) + 
              ",\"pwm\":" + String(pwm_percentage, 0) + 
              ",\"wh\":" + String(Wh, 1) + 
              ",\"load\":" + String(loadState) +
              ",\"algo\":" + String(MPPT_Mode ? 1 : 0) +
              ",\"mode\":" + String(output_Mode ? 1 : 0) +
              ",\"maxV\":" + String(voltageBatteryMax, 2) +
              ",\"minV\":" + String(voltageBatteryMin, 2) + "," +
              "\"maxCurrent\":" + String(currentCharging, 2) + 
              ",\"fanTemp\":" + String(temperatureFan) + 
              ",\"bnc\":" + String(BNC) + 
              ",\"flv\":" + String(FLV) + "}";
  
  server.send(200, "application/json", json);
  
}

void handleControl() {
  if (server.hasArg("load")) {
    pinValue2 = server.arg("load").toInt();
    dem = 2;  // Set to remote control mode
    server.send(200, "text/plain", "OK");
    
    Serial.print("Load control received: ");
    Serial.println(pinValue2);
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleSetPrice() {
  if (server.hasArg("price")) {
    electricalPrice = server.arg("price").toFloat();
    EEPROM.put(EE_PRICE_ADDR, electricalPrice);
    EEPROM.commit();
    server.send(200, "text/plain", "OK");
  }
}

void handleGetPrice() {
  String priceStr = String(electricalPrice, 2);
  server.send(200, "text/plain", priceStr);
}

void handleSetTime() {
  if (server.hasArg("h")) {
    setTime(server.arg("h").toInt(), server.arg("m").toInt(), server.arg("s").toInt(), 
            server.arg("d").toInt(), server.arg("mo").toInt(), server.arg("y").toInt());
    server.send(200, "text/plain", "OK");
  }
}

void handleLocalIP() {
  server.send(200, "text/plain", WiFi.localIP().toString());
}


void handleSetAlgorithm() {
  if (server.hasArg("mode")) {
    MPPT_Mode = server.arg("mode").toInt();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    output_Mode = server.arg("mode").toInt();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleSetMaxVoltage() {
  if (server.hasArg("value")) {
    voltageBatteryMax = server.arg("value").toFloat();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleSetMinVoltage() {
  if (server.hasArg("value")) {
    voltageBatteryMin = server.arg("value").toFloat();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleSetMaxCurrent() {
  if (server.hasArg("value")) {
    currentCharging = server.arg("value").toFloat();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleSetFanTemp() {
  if (server.hasArg("temp")) {
    temperatureFan = server.arg("temp").toInt();
    saveSettings();
    server.send(200, "text/plain", "OK");
  }
}

void handleFactoryReset() {
  factoryReset();
  Wh = 0;
  EEPROM.put(EE_WH_ADDR, Wh);
  EEPROM.commit();
  server.send(200, "text/plain", "OK");
}


void Wireless_Telemetry() {
  server.handleClient();
}