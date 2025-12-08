/*//----------------------------------------------------------------------
Made by Cavon Hajimiri 2024

Version 2: Added Capability of Sending Sensor Data to Server via Wifi 
Version 2 edits by Michael Zhang 9/26/2025

Ember Alert detects forest fires and alerts citizens nearby to allow for their safety as well as the quick elimination of the fire
This is a simple prototype to demonstrate the effectiveness of multiple small components 

*///----------------------------------------------------------------------

#include "WiFi.h"
#include <HTTPClient.h>

// ------------------- Wi-Fi credentials -------------------
const char* ssid = "Michael's iPhone"; // need to edit to specific WiFi
const char* password = "password123";

// ------------------- Server endpoint -------------------
String serverPath = "http://127.0.0.1:5050/receive"; // this will be different for different computers


#include "DHT.h" // Library for Temp and Humidity Sensor

//#define BLE_SETUP

#define DHT_PIN 2     // Digital pin connected to the DHT Sensor
#define IR_PIN 4      // Digital pin connected to the IR Sensor
#define LED_PIN 8     // Digital pin for testing LED
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//Current algo adjusts it
DHT dht(DHT_PIN, DHTTYPE);

int t1;
int t2;
int CMON_PIN = A0;

int IN1 = 3;
int IN2 = 6;


#include <SimpleKalmanFilter.h>
 
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);

// Serial output refresh time
const long SERIAL_REFRESH_TIME = 100;
long refresh_time;


#ifdef BLE_SETUP


  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>

  BLEServer *pServer = NULL;
  BLECharacteristic * pTxCharacteristic;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  //uint8_t txValue = 0;
  std::string rxValue = "";
  bool newBLEdata = false; 

  // See the following for generating UUIDs:
  // https://www.uuidgenerator.net/

  //Does work:
  #define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
  #define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
  #define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
  //*/
  /* 
  //Doesn't work:
  #define SERVICE_UUID           "d3d1dfd9-02c4-446b-ba9f-d70382dd8932" // UART service UUID
  #define CHARACTERISTIC_UUID_RX "d3d1dfdA-02c4-446b-ba9f-d70382dd8932"
  #define CHARACTERISTIC_UUID_TX "d3d1dfdB-02c4-446b-ba9f-d70382dd8932"
  //*/

  class MyServerCallbacks: public BLEServerCallbacks {
      void onConnect(BLEServer* pServer) {
        deviceConnected = true;
      };

      void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
      }
  };

  class MyCallbacks: public BLECharacteristicCallbacks {
      void onWrite(BLECharacteristic *pCharacteristic) {
        //std::string rxValue = pCharacteristic->getValue();
        rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0) {
          newBLEdata = true; 
          //Serial.println("*********");
          Serial.print("Received Value: ");
          for (int i = 0; i < rxValue.length(); i++)
            Serial.print(rxValue[i]);

          Serial.println();
          //Serial.println("*********");
        }
      }
  };


  void sendBLEString(std::string mystr){
    //Works:
    //pTxCharacteristic->setValue(&txValue2, 1);
    /*
    //Works:
    std::string numberString(numchararray, activeind);
    pTxCharacteristic->setValue(numberString);
    pTxCharacteristic->notify();
    //*/
    //For data transmission
    //Works: (direct conversion of number to string to char array)
    //int32_t mynum = value;
    //std::string mystr = std::to_string(mynum);
    char* mychararray = new char[mystr.length()];
    //mychararray[mystr.length()]=(char)linefeed; //add '\n' at the end
    for (int i = 0; i < mystr.length(); i++) {
      mychararray[i] = mystr[i];
    }
    std::string numberString(mychararray, mystr.length());
    //
    //Common part:
    pTxCharacteristic->setValue(numberString);
    pTxCharacteristic->notify();
    //delay(1000);
    //*/
  }
#endif



void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));

  Serial.println("Boot successful, entering setup...");

  // Set the output pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);

  t1 = millis();
  t2 = millis();

  dht.begin();

  //digitalWrite(IN1, LOW);
  //digitalWrite(IN2, HIGH);
  ///delay(2000);

  #ifdef BLE_SETUP
    // Create the BLE Device
    BLEDevice::init("EmberAlert");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Set power level:
    //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9);//some of the options dBm: N6, N3, N0, P3, P6, P9, P12, P15, P18
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); 
    //Serial.println(ESP_PWR_LVL_N6);

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                        
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_WRITE
                      );

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
  #endif

  // Connect Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Failed to connect to WiFi.");
    
  }

}

// Wifi Data Transmission Helper Function
void sendDataViaWifi(float h, float t, float f, float measured_value, float estimated_value, int IR_val) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverPath);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{";
    jsonData += "\"humidity\":" + String(h) + ",";
    jsonData += "\"temperature_c\":" + String(t) + ",";
    jsonData += "\"temperature_f\":" + String(f) + ",";
    jsonData += "\"ir_raw\":" + String(IR_val) + ",";
    jsonData += "\"ir_measured\":" + String(measured_value) + ",";
    jsonData += "\"ir_estimated\":" + String(estimated_value);
    jsonData += "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response: " + response);
    } else {
      Serial.print("HTTP Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}

void loop() {
  // Wait a few seconds between measurements.
  //digitalWrite(LED_PIN, HIGH);
  //delay(2000);
  // int IR_val = analogRead(IR_PIN);
  // //Serial.println(val);
  // //digitalWrite(LED_PIN, LOW);

  // if (millis()-t1 >= 250){
  //   //int CMON_out = analogRead(CMON_PIN);
  //   //Serial.println(CMON_out);
  //   // Reading temperature or humidity takes about 250 milliseconds!
  //   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //   float h = dht.readHumidity();
  //   // Read temperature as Celsius (the default)
  //   float t = dht.readTemperature();
  //   // Read temperature as Fahrenheit (isFahrenheit = true)
  //   float f = dht.readTemperature(true);

  //   // Check if any reads failed and exit early (to try again).
  //   if (isnan(h) || isnan(t) || isnan(f)) {
  //     Serial.println(F("Failed to read from DHT sensor!"));
  //     return;
  //   }


  //   // Compute heat index in Fahrenheit (the default)
  //   float hif = dht.computeHeatIndex(f, h);
  //   // Compute heat index in Celsius (isFahreheit = false)
  //   float hic = dht.computeHeatIndex(t, h, false);

  //   // read a reference value from A0 and map it from 0 to 100
  //   float real_value = hif*4.8;//Rough Approximation to make the signals comprable
    
  //   // IR Analog Value
  //   float measured_value = analogRead(A0)/1024.0 * 100.0;

  //   // calculate the estimated value with Kalman Filter function we defined
  //   float estimated_value = simpleKalmanFilter.updateEstimate(measured_value);

  //   // //Digital IR_val is 1 when no flame and 0 when there is a flame
  //   // if (estimated_value >= Kalman_Threshold){//Checking Kalman filter value 
  //   //   digitalWrite(LED_PIN, HIGH);
  //   //   digitalWrite(IN1, LOW);
  //   //   digitalWrite(IN2, HIGH);
  //   // }
  //   // else{
  //   //   digitalWrite(LED_PIN, LOW);
  //   //   digitalWrite(IN1, HIGH);
  //   //   digitalWrite(IN2, HIGH);
  //   // }
  
  //   Serial.print(real_value,4);
  //   Serial.print(",");
  //   Serial.print(measured_value,4);
  //   Serial.print(",");
  //   Serial.print(estimated_value,4);
  //   Serial.println();

  //   // send the data to server
  //   sendDataViaWifi(h, t, f, measured_value, estimated_value, IR_val);

  //   /*
  //   Serial.print(F("Humidity: "));
  //   Serial.print(h);
  //   Serial.print(F("%  Temperature: "));
  //   Serial.print(t);
  //   Serial.print(F("°C "));
  //   Serial.print(f);
  //   Serial.print(F("°F  Heat index: "));
  //   Serial.print(hic);
  //   Serial.print(F("°C "));
  //   Serial.print(hif);
  //   Serial.println(F("°F"));
  //   //*/
  //   t1 = millis();
  }

  #ifdef BLE_SETUP
  if(millis()-t2 >= 500){
    if (deviceConnected) {      
      //
      //int32_t num_peaks_to_transmit = 3;
      //Convert the arrays of data to comma separated strings
      std::string mystr = "";
      std::string addedstr = std::to_string(IR_val);
      mystr = mystr + addedstr + "," ; // concatenate strings
      //std::string addedstr1 = std::to_string(t2);
      //mystr = mystr + addedstr1 + "," ; // concatenate strings
      mystr.replace(mystr.length()-1,1,"\n");
      //}
      //Send the string to the BLE device
      sendBLEString(mystr);

      //DDD: Delay for plotting pon Bluefruitconnect App: DDD
      //Needs adjustment later
      delay(30);
    }
    t2 = millis();
  }
  #endif

}
