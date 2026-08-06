
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


// --- Pin Configurations ---
const uint8_t TRIG_PIN = 2;
const uint8_t ECHO_PIN = 3;


// --- Wi-Fi Credentials ---
const char* WIFI_SSID     = "Vodafone-5A8C"; // Replace with your Wi-Fi SSID
const char* WIFI_PASSWORD = "Naresh123*";

// Replace with your Webhook.site URL for testing
const char* UPLOAD_URL    = "https://webhook.site/4bb4961d-7b7d-4cf4-b7e5-50432a2a64f7";

// --- Threshold & Settings ---
const float TRIGGER_DISTANCE_CM = 20.0; // Distance in cm to trigger servo
#define SOUND_SPEED 0.0343

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set I2C address (usually 0x27 or 0x3F), 16 columns, 2 rows
bool photoTaken = false; // Flag to ensure photo is taken only once per detection



void setup() {
  Serial.begin(115200);

  initializeUltrasonicSensor(); // Initialize Ultrasonic 
  initializeServo(); // Initialize Servo Pin
  initializeLCD(); // Initialize LCD
  connectWiFi(WIFI_SSID, WIFI_PASSWORD); // Connect to Wi-Fi

  // Initialize Camera
  // lcd.print("Init Camera...");
  if (initCamera()) {
    Serial.println("Camera initialized successfully!");
    lcd.setCursor(0, 1);
    lcd.print("Camera Ready!   ");
  } else {
    Serial.println("Camera initialization failed!");
    lcd.setCursor(0, 1);
    lcd.print("Camera Failed!  ");
  }

  // initialize Fingerprint Sensor

  if (initFingerprint()) {
    Serial.println("Fingerprint Module Ready!");
  } else {
    Serial.println("Fingerprint Initialization Failed!");
  }

  Serial.println("--- ESP32-S3 Obstacle Detection System Ready ---");
}

void loop() {
  float distance = readDistance();
  lcd.setCursor(0, 0);

  // fingerprint enrollment logic (change this to a button press or other trigger in a real application)
  if (Serial.available() > 0) {
    int enrollID = Serial.parseInt();
    if (enrollID > 0 && enrollID < 128) {
      enrollFingerprint(enrollID);
    } else {
      Serial.println("Invalid ID! Please enter a number between 1 and 127.");
    }
  }
  if (distance < 0) {
    Serial.println("Distance: Out of Range / No Echo");
    lcd.print("Dist: Out Range ");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    lcd.print("Dist: ");
    lcd.print(distance, 1);
    lcd.print(" cm   ");

    lcd.setCursor(0, 1);
    // Check if an object is within the target distance threshold
    if (distance > 0 && distance <= TRIGGER_DISTANCE_CM) {
      Serial.println("⚠️ Object Detected! Moving Servo to 180°...");
      rotateServoToAngle(180);
      if(!photoTaken) {
        lcd.setCursor(0, 0);
        lcd.print("Capturing Photo... ");

        delay(1000); // Wait for 1 second to allow the servo to reach position
        if (captureAndUpload(UPLOAD_URL)) {
          Serial.println("Photo captured and uploaded successfully!");
          lcd.setCursor(0, 1);
          lcd.print("Photo Uploaded!   ");
        } else {
          Serial.println("Failed to capture or upload photo.");
          lcd.setCursor(0, 1);
          lcd.print("Upload Failed!    ");
        }

        // collects the finger print ID after taking the photo
        int fingerID = getFingerprintID();
        if (fingerID > 0) {
        
        Serial.printf("Authorized User Detected! ID: %d\n", fingerID);
    // Trigger your servo or upload photo logic here
      
        } else {
          Serial.println("Unauthorized User or No Fingerprint Detected!");
        }
        photoTaken = true; // Set flag to true after taking photo
        delay(1000); // Wait for 1 second to allow the user to see the message
      }
      lcd.print("STATUS: CLOSE!  ");


    } else {
      Serial.println("Path clear. Returning Servo to 0°...");
      rotateServoToAngle(0);
      photoTaken = false; // Reset flag when path is clear
      lcd.print("STATUS: CLEAR   ");
    }
  }

  delay(200); // Check distance 5 times per second
}