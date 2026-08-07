#include "wifi_connect.h"
#include <WiFi.h>
#include "config.h"

// --- Wi-Fi Credentials ---
const char *WIFI_SSID = "Vodafone-5A8C"; // Replace with your Wi-Fi SSID
const char *WIFI_PASSWORD = "Naresh123*";

void connectWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    sendToLcd("WiFi....", "Connected");

    delay(2000);
  }
  else
  {
    Serial.println("\nWiFi Connection Failed!");
    sendToLcd("WiFi....", "Failed");
    delay(2000);
  }
}