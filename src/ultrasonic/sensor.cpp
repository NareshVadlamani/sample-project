#include <Arduino.h>
#include "ultrasonic_sensor.h"
#include "config.h"

bool photoTaken = false;
bool activePresence = false;

// Function to read distance from HC-SR04 with a timeout
float readDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0)
    {
        return -1.0;
    }
    return (duration * SOUND_SPEED) / 2.0;
}

void resetUltrasonicPresence()
{
    activePresence = false;
}

void readUltrasonicSensor()
{
    float distance = readDistance();

    if (distance > 0 && distance <= TRIGGER_DISTANCE_CM)
    {
        if (!activePresence)
        {
            activePresence = true;
            SystemEvent ev = {EVENT_PERSON_NEAR, {0}};
            xQueueSend(xEventQueue, &ev, 0);
        }
    }
    else if (distance > RELEASE_DISTANCE_CM || distance <= 0)
    {

        activePresence = false;
        SystemEvent ev = {EVENT_PERSON_LEFT, {0}};
        xQueueSend(xEventQueue, &ev, 0);
    }
}

void TaskUltrasonicSensor(void *pvParameters)
{
    for (;;)
    {
        readUltrasonicSensor();
        vTaskDelay(pdMS_TO_TICKS(150)); // Yields CPU for 150ms
    }
}

void initializeUltrasonicSensor()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    xTaskCreatePinnedToCore(
        TaskUltrasonicSensor, "UltrasonicSensorTask", 4096, NULL, 1, NULL, 0);
}
