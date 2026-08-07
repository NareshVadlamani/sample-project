#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

void initializeUltrasonicSensor();
float readDistance();
void readUltrasonicSensor();

#endif