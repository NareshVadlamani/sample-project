#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <ESP32Servo.h>

// Pins
#define I2C_SDA_PIN 41    // lcd SDA pin
#define I2C_SCL_PIN 42    // lcd SCL pin
#define TRIG_PIN 2        // ultrasonic sensor trigger pin
#define ECHO_PIN 3        // ultrasonic sensor echo pin
#define FP_RX_PIN 20      // fingerprint sensor RX pin (connects to sensor TX)
#define FP_TX_PIN 21      // fingerprint sensor TX pin (connects to sensor RX)
#define SERVO_PIN 1       // servo control pin
#define ENROLL_BTN_PIN 14 // fingerprint enrollment button pin

#define SOUND_SPEED 0.0343
const float TRIGGER_DISTANCE_CM = 30.0;
const float RELEASE_DISTANCE_CM = 50.0;

// System States & Event Types
enum SystemState
{
    STATE_IDLE,
    STATE_AWAITING_FINGER,
    STATE_ACCESS_GRANTED,
    STATE_ACCESS_DENIED
};

enum EventType
{
    EVENT_PERSON_NEAR,
    EVENT_PERSON_LEFT,
    EVENT_FINGER_MATCHED,
    EVENT_FINGER_FAILED
};

struct SystemEvent
{
    EventType type;
    int payload;
};

struct LcdMessage
{
    char line1[17];
    char line2[17];
    bool clearFirst;
};

// Global Hardware & Queue Handles
extern LiquidCrystal_I2C lcd;
extern Adafruit_Fingerprint finger;
extern Servo doorServo;

extern QueueHandle_t xEventQueue;
extern QueueHandle_t xLcdQueue;

extern volatile bool isFingerprintEnabled;

// Shared Display Helper Function
void sendToLcd(const char *l1, const char *l2, bool clear = false);

#endif