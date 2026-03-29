// ===== LIBRARIES =====
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ===== WIFI CREDENTIALS =====
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ===== PIN DEFINITIONS =====
#define DHT_PIN     4       // DHT11/DHT22 data pin
#define DHT_TYPE    DHT22   // Change to DHT11 if using DHT11
#define IR_PIN      34      // Infrared sensor analog/digital pin (ADC-capable)
#define GAS_PIN     35      // MQ-series gas sensor analog pin (ADC-capable)

// ===== OBJECTS =====
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

// ===== SENSOR DATA (global cache) =====
float temperature = 0.0;
float humidity    = 0.0;
int   gasValue    = 0;
int   irValue     = 0;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 2000; // DHT needs ~2s between reads

// ===== WEBPAGE =====
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Sensor Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: #f4f4f4;
      margin: 0;
      padding: 20px;
    }
    h2 { color: #333; }
    .chart-wrapper {
      background: white;
      border-radius: 12px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      padding: 16px;
      max-width: 620px;
      margin: 20px auto;
    }
    canvas { width: 100% !important; }
    .readings {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 16px;
      margin: 20px auto;
      max-width: 700px;
    }
    .card {
      background: white;
      border-radius: 12px;
      padding: 16px 24px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      min-width: 130px;
    }
    .card .label { font-size: 12px; color: #888; text-transform: uppercase; }
    .card .value { font-size: 28px; font-weight: bold; margin-top: 4px; }
    #tempVal  { color: #e74c3c; }
    #humVal   { color: #2980b9; }
    #gasVal   { color: #27ae60; }
    #irVal    { color: #e67e22; }
  </style>
</head>
<body>
  <h2>ESP32 Sensor Dashboard</h2>

  <!-- Live value cards -->
  <div class="readings">
    <div class="card">
      <div class="label">Temperature</div>
      <div class="value" id="tempVal">--</div>
      <div class="label">°C</div>
    </div>
    <div class="card">
      <div class="label">Humidity</div>
      <div class="value" id="humVal">--</div>
      <div class="label">%</div>
    </div>
    <div class="card">
      <div class="label">Gas</div>
      <div class="value" id="gasVal">--</div>
      <div class="label">ADC (0–4095)</div>
    </div>
    <div class="card">
      <div class="label">Infrared</div>
      <div class="value" id="irVal">--</div>
      <div class="label">ADC (0–4095)</div>
    </div>
  </div>

  <!-- Charts -->
  <div class="chart-wrapper"><canvas id="tempChart"></canvas></div>
  <div class="chart-wrapper"><canvas id="humChart"></canvas></div>
  <div class="chart-wrapper"><canvas id="gasChart"></canvas></div>
  <div class="chart-wrapper"><canvas id="irChart"></canvas></div>

  <script>
    function makeChart(ctx, label, color) {
      return new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: label,
            data: [],
            borderColor: color,
            backgroundColor: color + '22',
            fill: true,
            tension: 0.3,
            pointRadius: 3
          }]
        },
        options: {
          animation: false,
          plugins: { legend: { display: true } },
          scales: { y: { beginAtZero: false } }
        }
      });
    }

    const tempChart = makeChart(document.getElementById("tempChart"), "Temperature (°C)", "#e74c3c");
    const humChart  = makeChart(document.getElementById("humChart"),  "Humidity (%)",     "#2980b9");
    const gasChart  = makeChart(document.getElementById("gasChart"),  "Gas (ADC)",        "#27ae60");
    const irChart   = makeChart(document.getElementById("irChart"),   "Infrared (ADC)",   "#e67e22");

    function addData(chart, label, value) {
      if (chart.data.labels.length > 30) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
      }
      chart.data.labels.push(label);
      chart.data.datasets[0].data.push(value);
      chart.update();
    }

    function fetchAll() {
      fetch("/sensors")
        .then(resp => resp.json())
        .then(data => {
          const now = new Date().toLocaleTimeString();
          addData(tempChart, now, data.temperature);
          addData(humChart,  now, data.humidity);
          addData(gasChart,  now, data.gas);
          addData(irChart,   now, data.ir);

          document.getElementById("tempVal").textContent = data.temperature.toFixed(1);
          document.getElementById("humVal").textContent  = data.humidity.toFixed(1);
          document.getElementById("gasVal").textContent  = data.gas;
          document.getElementById("irVal").textContent   = data.ir;
        })
        .catch(err => console.error("Fetch error:", err));
    }

    setInterval(fetchAll, 2000);
    fetchAll(); // run immediately on load
  </script>
</body>
</html>
)rawliteral";

// ===== ROUTE: serve dashboard =====
void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

// ===== ROUTE: return all sensor data as JSON =====
void handleSensors() {
  // Re-read gas and IR every request (fast analog reads)
  gasValue = analogRead(GAS_PIN);
  irValue  = analogRead(IR_PIN);

  String json = "{";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"humidity\":"    + String(humidity, 2)    + ",";
  json += "\"gas\":"         + String(gasValue)       + ",";
  json += "\"ir\":"          + String(irValue);
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// ===== ROUTE: 404 =====
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Register routes
  server.on("/",        handleRoot);
  server.on("/sensors", handleSensors);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server started.");
}

// ===== LOOP =====
void loop() {
  server.handleClient();

  // Read DHT on interval (it's slow — max ~0.5 Hz for DHT11, ~1 Hz for DHT22)
  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temperature = t;
    else Serial.println("DHT temperature read failed");

    if (!isnan(h)) humidity = h;
    else Serial.println("DHT humidity read failed");
  }
}
