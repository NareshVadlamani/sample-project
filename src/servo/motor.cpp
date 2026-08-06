#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

void initializeServo()
{
  // Initialize Servo Pin
  const uint8_t SERVO_PIN = 1;
  myServo.attach(SERVO_PIN);

  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50); // Standard 50Hz servo
  myServo.attach(SERVO_PIN, 500, 2400);

  // Start servo at 0 degrees
  myServo.write(0);
}

void rotateServoToAngle(int angle)
{
  if (angle < 0 || angle > 180)
  {
    Serial.println("Invalid angle! Please provide an angle between 0 and 180 degrees.");
    return;
  }
  myServo.write(angle);
}