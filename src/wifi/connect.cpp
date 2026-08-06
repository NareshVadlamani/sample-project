#include "wifi_connect.h"
#include <WiFi.h>
#include "lcd_helper.h"

// --- Wi-Fi Credentials ---
const char *WIFI_SSID = "Vodafone-5A8C"; // Replace with your Wi-Fi SSID
const char *WIFI_PASSWORD = "Naresh123*";

void connectWiFi()
{
  safeLcdWrite(0, 0, true, "Connecting WiFi ");
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    safeLcdWrite(attempts % 16, 1, true, ".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    safeLcdWrite(0, 0, true, "WiFi Connected!");
    safeLcdWrite(0, 1, false, "IP: %s", WiFi.localIP().toString().c_str());
    delay(2000);
  }
  else
  {
    Serial.println("\nWiFi Connection Failed!");
    safeLcdWrite(0, 0, true, "WiFi Failed!");
    delay(2000);
  }
}