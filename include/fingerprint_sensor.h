#ifndef FINGERPRINT_SENSOR_H
#define FINGERPRINT_SENSOR_H

#include <Arduino.h>

// Pins for ESP32-S3 HardwareSerial 2
const uint8_t FP_RX_PIN = 20; // Connects to Sensor TX
const uint8_t FP_TX_PIN = 21; // Connects to Sensor RX

// Initialize the fingerprint sensor on Serial2
bool initFingerprint();

// Scan for a finger and return the matched ID (returns -1 if no match)
int getFingerprintID();
bool enrollFingerprint(uint8_t id);

#endif // FINGERPRINT_SENSOR_H