#ifndef FINGERPRINT_SENSOR_H
#define FINGERPRINT_SENSOR_H

#include <Arduino.h>

// Pins for ESP32-S3 HardwareSerial 2
const uint8_t FP_RX_PIN = 20; // Connects to Sensor TX
const uint8_t FP_TX_PIN = 21; // Connects to Sensor RX
const uint8_t ENROLL_BTN_PIN = 19;

// Initialize the fingerprint sensor on Serial2
void initFingerprint();

int getFingerprintID(); // Scan for a finger and return the matched ID (returns -1 if no match)

#endif // FINGERPRINT_SENSOR_H