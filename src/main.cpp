#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define AOUT_PIN A0   // Soil moisture sensor pin
#define RELAY_PIN D1  // Relay module pin for water pump

// LED Pins
#define WIFI_LED D2  // Blue LED (WiFi status)
#define API_LED D3   // Red LED (API status)
#define SYS_LED D4   // Green LED (System working well)

const char* ssid = "GIHANGA AI";
const char* password = "GIHANGA1";
const char* server = "http://192.168.1.14:5000/save_data"; // Your PHP API endpoint

void setup() {
    Serial.begin(9600);

    // Set up pins
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(WIFI_LED, OUTPUT);
    pinMode(API_LED, OUTPUT);
    pinMode(SYS_LED, OUTPUT);

    digitalWrite(RELAY_PIN, HIGH); // Ensure pump is off initially
    digitalWrite(SYS_LED, HIGH);   // System is ON
    digitalWrite(API_LED, LOW);    // Default API LED OFF
    digitalWrite(WIFI_LED, LOW);   // Default WiFi LED OFF

    // Connecting to WiFi
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(WIFI_LED, !digitalRead(WIFI_LED)); // Blink while connecting
        delay(1000);
        Serial.print(".");
    }

    digitalWrite(WIFI_LED, HIGH);  // WiFi connected
    Serial.println("\nConnected to WiFi!");
}

void loop() {
    int moisture = analogRead(AOUT_PIN); // Read soil moisture
    Serial.print("Moisture Level: ");
    Serial.println(moisture);

    // Control water pump based on soil moisture
    if (moisture < 400) { 
        digitalWrite(RELAY_PIN, LOW); // Turn ON pump
    } else {
        digitalWrite(RELAY_PIN, HIGH); // Turn OFF pump
    }

    // Check WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(WIFI_LED, HIGH); // WiFi is connected
        WiFiClient client;
        HTTPClient http;
        
        String url = String(server) + "?moisture=" + moisture;
        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Data sent to server!");
            digitalWrite(API_LED, HIGH);  // Data sent successfully
        } else {
            Serial.println("Failed to send data.");
            for (int i = 0; i < 3; i++) { // Blink API LED 3 times
                digitalWrite(API_LED, HIGH);
                delay(300);
                digitalWrite(API_LED, LOW);
                delay(300);
            }
        }
        http.end();
    } else {
        digitalWrite(WIFI_LED, LOW); // WiFi disconnected
    }

    delay(5000); // Delay before next reading
}
