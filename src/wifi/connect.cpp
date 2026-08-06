#include "wifi_connect.h"
#include <WiFi.h>
#include "LiquidCrystal_I2C.h"

// Reference global LCD object declared in main.cpp
extern LiquidCrystal_I2C lcd;

void connectWiFi(const char* ssid, const char* password) {
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi ");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
    attempts++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nWiFi Connection Failed!");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    delay(2000);
  }
  lcd.clear();
}