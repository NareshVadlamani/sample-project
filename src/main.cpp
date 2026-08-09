
#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include "wifi_connect.h"
#include "camera_uploader.h"
#include "fingerprint_sensor.h"
#include "ultrasonic_sensor.h"
#include "servo_motor.h"
#include "lcd_screen.h"
#include "fsm_controler.h"
#include "config.h"
#include "network_add_logs.h"

QueueHandle_t xEventQueue = NULL;
QueueHandle_t xLcdQueue = NULL;
QueueHandle_t xLogQueue = NULL;

Servo doorServo;

volatile bool isFingerprintEnabled = false;

void setup()
{
  Serial.begin(115200);
  uint32_t startWait = millis();
  while (!Serial && (millis() - startWait < 3000))
  {
    delay(10);
  }

  Serial.println("\n=================================");
  Serial.println("  ESP32-S3 SYSTEM INITIALIZED    ");
  Serial.println("=================================");
  delay(1000); // Allow time for Serial Monitor to initialize

  xEventQueue = xQueueCreate(10, sizeof(SystemEvent));
  xLcdQueue = xQueueCreate(5, sizeof(LcdMessage));
  xLogQueue = xQueueCreate(5, sizeof(LogPayload));

  connectWiFi(); // Connect to Wi-Fi
  initCamera();  // Initialize Camera

  delay(1000); // Allow for init

  initializeLCD();   // Initialize LCD
  initFingerprint(); // Initialize Fingerprint Sensor
  initializeServo(); // Initialize Servo Pin

  initializeUltrasonicSensor(); // Initialize Ultrasonic

  xTaskCreatePinnedToCore(TaskSystemManager, "FSM_Task", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(TaskNetworkLogger, "NETWORK_TASK", 4096, NULL, 1, NULL, 0);

  Serial.println("--- ESP32-S3 Obstacle Detection System Ready ---");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms to avoid flooding the serial output
}