#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

void initializeServo()
{
  doorServo.attach(SERVO_PIN);

  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  doorServo.setPeriodHertz(50); // Standard 50Hz servo
  doorServo.attach(SERVO_PIN, 500, 2400);

  doorServo.write(0);
}

void rotateServoToAngle(int angle)
{
  if (angle < 0 || angle > 180)
  {
    Serial.println("Invalid angle! Please provide an angle between 0 and 180 degrees.");
    return;
  }
  doorServo.write(angle);
}