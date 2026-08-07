#include <Arduino.h>
#include "ultrasonic_sensor.h"
#include "camera_uploader.h"
#include "servo_motor.h"
#include "fingerprint_sensor.h"
#include "lcd_helper.h"

const uint8_t TRIG_PIN = 2;
const uint8_t ECHO_PIN = 3;

bool photoTaken = false;

#define SOUND_SPEED 0.0343
const float TRIGGER_DISTANCE_CM = 20.0; // Distance in cm to trigger servo

void TaskUltrasonicSensor(void *pvParameters)
{
    for (;;)
    {
        readUltrasonicSensor();
        vTaskDelay(pdMS_TO_TICKS(100)); // Yields CPU for 100ms
    }
}

void initializeUltrasonicSensor()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    xTaskCreatePinnedToCore(
        TaskUltrasonicSensor, "UltrasonicSensorTask", 4096, NULL, 1, NULL, 0);
}

// Function to read distance from HC-SR04 with a timeout
float readDistance()
{
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
    if (duration == 0)
    {
        return -1.0;
    }

    // Calculate distance in cm
    float distance = (duration * SOUND_SPEED) / 2.0;

    return distance;
}

void readUltrasonicSensor()
{
    rotateServoToAngle(0);
    float distance = readDistance();

    safeLcdWrite(0, 0, false, "Distance: %.2f cm", distance);
    if (distance > 0 && distance <= TRIGGER_DISTANCE_CM)
    {
        rotateServoToAngle(180);
        if (!photoTaken)
        {
            triggerCameraUpload();
            photoTaken = true;
        }
        safeLcdWrite(0, 1, false, "place finger...");
        delay(2000); // Wait for user to place finger
        int fingerID = getFingerprintID();
        safeLcdWrite(0, 1, false, "Fingerprint ID: %d", fingerID);
        if (fingerID > 0)
        {
            Serial.printf("Authorized User Detected! ID: %d\n", fingerID);
            safeLcdWrite(0, 1, false, "Gate opened for ID: %d", fingerID);
            rotateServoToAngle(90); // Reset servo after access
        }
        else
        {
            Serial.println("Unauthorized User or No Fingerprint Detected!");
            safeLcdWrite(0, 1, false, "Unauthorized...");
        }
    }
}
