#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Replace with the receiver's MAC address
uint8_t broadcastAddress[] = {0x64, 0xE8, 0x33, 0x80, 0x4A, 0xC4};

// Callback for send status
void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give Serial time to initialize

    WiFi.mode(WIFI_STA); // Must be STA mode for ESP-NOW

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        while (true); // Stop if init fails
    }

    // Register send callback
    esp_now_register_send_cb(onDataSent);

    // Register the receiver as a peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        while (true); // Stop if peer can't be added
    }

    Serial.println("ESP-NOW Sender Ready");
}

void loop() {
    const char *message = "Hello ESP-NOW";
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, strlen(message));
   
    if (result == ESP_OK) {
        Serial.println("Message sent");
    } else {
        Serial.printf("Error sending message: %d\n", result);
    }

    delay(2000); // Send every 2 seconds
}