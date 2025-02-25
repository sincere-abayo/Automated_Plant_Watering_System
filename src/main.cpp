#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define AOUT_PIN A0   // Soil moisture sensor pin
#define RELAY_PIN D6  // Relay module pin for water pump

// LED Pins
#define WIFI_LED D2  // Blue LED (WiFi status)
#define API_LED D3   // Red LED (API status)
#define SYS_LED D4   // Green LED (System working well)

const char* ssid = "bjaynet";
const char* password = "bjay1010..";
const char* server = "http://192.168.43.51:5000/save_data"; // Your PHP API endpoint

void setup() {
    Serial.begin(9600);
    Serial.println("\nInitializing Automated Plant Watering System...");
    
    // Set up pins
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(API_LED, OUTPUT);
    pinMode(SYS_LED, OUTPUT);

    // Connect to WiFi
    Serial.println("\nConnecting to WiFi...");
    pinMode(WIFI_LED, OUTPUT);
    digitalWrite(WIFI_LED, LOW);  // Start with LED OFF

    WiFi.begin(ssid, password);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(WIFI_LED, !digitalRead(WIFI_LED)); // Blink LED while connecting
        Serial.print(".");
        delay(1000);
        
        attempt++;
        if (attempt > 15) { // Stop after 15 attempts
            Serial.println("\n❌ WiFi Connection Failed!");
            digitalWrite(WIFI_LED, LOW); // Keep LED OFF if not connected
            return;
        }
    }

    digitalWrite(WIFI_LED, HIGH);  // WiFi Connected, LED ON
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("Device IP Address: ");
    Serial.println(WiFi.localIP());

    digitalWrite(RELAY_PIN, HIGH); // Ensure pump is off initially
    digitalWrite(SYS_LED, HIGH);   // System is ON
    digitalWrite(API_LED, LOW);    // Default API LED OFF
}

void loop() {
    // Keep checking if WiFi is still connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi Disconnected!");
        digitalWrite(WIFI_LED, LOW); // Turn off LED if disconnected
    } else {
        digitalWrite(WIFI_LED, HIGH); // Keep LED ON if connected
    }

    int moisture = analogRead(AOUT_PIN); // Read soil moisture
    
    Serial.print("\nMoisture Level: ");
    Serial.println(moisture);
    

    // Control water pump based on soil moisture
    if (moisture < 400) { 
        digitalWrite(RELAY_PIN, LOW); // Turn ON pump
        Serial.println("Water Pump: ON (Soil is dry)");
    } else {
        digitalWrite(RELAY_PIN, HIGH); // Turn OFF pump
        Serial.println("Water Pump: OFF (Soil is moist)");
    }

    // Send data to server
    WiFiClient client;
    HTTPClient http;
    
    String url = String(server) + "?moisture=" + moisture;
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        Serial.println("✅ Data sent to server successfully!");
        digitalWrite(API_LED, HIGH);  // Data sent successfully
    } else {
        Serial.println("❌ Failed to send data to server.");
        for (int i = 0; i < 3; i++) { // Blink API LED 3 times
            digitalWrite(API_LED, HIGH);
            delay(300);
            digitalWrite(API_LED, LOW);
            delay(300);
        }
    }
    http.end();

    delay(15000); // Delay before next reading
}