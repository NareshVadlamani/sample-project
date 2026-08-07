
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

QueueHandle_t xEventQueue = NULL;
QueueHandle_t xLcdQueue = NULL;
Servo doorServo;

volatile bool isFingerprintEnabled = false;

void setup()
{
  Serial.begin(115200);
  delay(1000); // Allow time for Serial Monitor to initialize

  xEventQueue = xQueueCreate(10, sizeof(SystemEvent));
  xLcdQueue = xQueueCreate(5, sizeof(LcdMessage));

  initializeLCD();   // Initialize LCD
  initFingerprint(); // Initialize Fingerprint Sensor
  initializeServo(); // Initialize Servo Pin

  initializeUltrasonicSensor(); // Initialize Ultrasonic
  connectWiFi();                // Connect to Wi-Fi
  initCamera();                 // Initialize Camera

  xTaskCreatePinnedToCore(TaskSystemManager, "FSM_Task", 4096, NULL, 3, NULL, 1);

  Serial.println("--- ESP32-S3 Obstacle Detection System Ready ---");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms to avoid flooding the serial output
}