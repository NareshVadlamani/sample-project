#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

void initializeUltrasonicSensor();
void TaskUltrasonic(void *pvParameters);

#endif