#ifndef FINGERPRINT_SENSOR_H
#define FINGERPRINT_SENSOR_H

#include <Arduino.h>

void initFingerprint();

int getFingerprintID();                   // Scan for a finger and return the matched ID (returns -1 if no match)
void TaskFingerprint(void *pvParameters); // Task to continuously scan for fingerprints

#endif // FINGERPRINT_SENSOR_H