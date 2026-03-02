#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Callback function when data is received
void onDataReceived(const uint8_t *macAddr, const uint8_t *data, int len) {
  // Print sender MAC
  Serial.print("Received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", macAddr[i]);
    if (i < 5) Serial.print(":");
  }

  // Print message
  Serial.print(" | Data: ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)data[i]);  // assuming data is a string
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // optional: give serial some time

  // Set ESP32 to station mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback for receiving data
  esp_now_register_recv_cb(onDataReceived);

  Serial.println("ESP-NOW Receiver Ready");
}

void loop() {

}