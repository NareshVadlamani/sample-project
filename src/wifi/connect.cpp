#include "wifi_connect.h"
#include <WiFi.h>
#include "config.h"

// --- Wi-Fi Credentials ---
const char *WIFI_SSID = "Vodafone-5A8C"; // Replace with your Wi-Fi SSID
const char *WIFI_PASSWORD = "Naresh123*";

void setupWifiLED()
{
  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW); // Start OFF

  // Triggered automatically when ESP32 gets an IP address
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
               {
        digitalWrite(WIFI_LED_PIN, HIGH);
        Serial.println("[Wi-Fi] Connected! LED ON"); }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  // Triggered automatically if Wi-Fi drops or disconnects
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
               {
        digitalWrite(WIFI_LED_PIN, LOW);
        Serial.println("[Wi-Fi] Disconnected! LED OFF"); }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void connectWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  setupWifiLED();

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    delay(2000);
  }
  else
  {
    Serial.println("\nWiFi Connection Failed!");
    delay(2000);
  }
}