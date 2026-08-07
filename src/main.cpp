
#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "wifi_connect.h"
#include "camera_uploader.h"
#include "fingerprint_sensor.h"
#include "ultrasonic_sensor.h"
#include "servo_motor.h"
#include "lcd_screen.h"
#include "lcd_helper.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set I2C address (usually 0x27 or 0x3F), 16 columns, 2 rows

SemaphoreHandle_t lcdMutex; // Mutex for LCD access

void setup()
{
  Serial.begin(115200);
  lcdMutex = xSemaphoreCreateMutex(); // Create mutex for LCD access

  initializeUltrasonicSensor(); // Initialize Ultrasonic
  initializeServo();            // Initialize Servo Pin
  initializeLCD();              // Initialize LCD
  connectWiFi();                // Connect to Wi-Fi
  initCamera();                 // Initialize Camera
  initFingerprint();            // Initialize Fingerprint Sensor

  Serial.println("--- ESP32-S3 Obstacle Detection System Ready ---");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms to avoid flooding the serial output
}