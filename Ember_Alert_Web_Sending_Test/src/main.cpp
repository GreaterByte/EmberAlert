#include <WiFi.h>
#include <WebServer.h>

// ====== CHANGE THESE ======
const char* ssid = "techer";
const char* password = "theovictor";
// ==========================

WebServer server(80);

volatile unsigned long counter = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 1000;

// HTML page with auto-updating JavaScript
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Counter</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial; 
            text-align: center; 
            margin-top: 50px; 
        }
        h1 { font-size: 48px; }
    </style>
</head>
<body>
    <h1 id="number">0</h1>

    <script>
        setInterval(function() {
            fetch("/value")
            .then(response => response.text())
            .then(data => {
                document.getElementById("number").innerHTML = data;
            });
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", webpage);
}

void handleValue() {
    server.send(200, "text/plain", String(counter));
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/value", handleValue);

    server.begin();
}

void loop() {
    server.handleClient();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        counter++;
    }
}