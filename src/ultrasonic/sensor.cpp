#include <Arduino.h>

const uint8_t TRIG_PIN = 2;
const uint8_t ECHO_PIN = 3;

#define SOUND_SPEED 0.0343

void initializeUltrasonicSensor() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

// Function to read distance from HC-SR04 with a timeout
float readDistance() {
    // Clear the TRIG pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Send a 10us trigger pulse
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read ECHO pin with 30,000us (~5m max range) timeout to prevent freezing
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    // If no echo returned or out of range
    if (duration == 0) {
        return -1.0; 
    }

    // Calculate distance in cm
    return (duration * SOUND_SPEED) / 2.0;
}