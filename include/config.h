#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <ESP32Servo.h>

// --- I2C LCD ---
#define I2C_SDA_PIN 41 // lcd SDA pin
#define I2C_SCL_PIN 42 // lcd SCL pin

// --- Ultrasonic Sensor ---
#define TRIG_PIN 3  // ultrasonic sensor trigger pin
#define ECHO_PIN 14 // ultrasonic sensor echo pin

// --- Fingerprint Sensor (UART) ---
#define FP_RX_PIN 20 // fingerprint RX (connects to sensor TX) - avoids USB D+
#define FP_TX_PIN 21 // fingerprint TX (connects to sensor RX)

// --- Actuators & Indicators ---
#define SERVO_PIN 2      // servo control pin (Moved from 47 -> 2 for clean PWM)
#define BUZZER_PIN 1     // buzzer pin (Clean LEDC PWM output)
#define WIFI_LED_PIN 19  // Wi-Fi status LED (Moved from 39 -> 19)
#define ENROLL_BTN_PIN 0 // Built-in BOOT button or external button to GND

#define SOUND_SPEED 0.0343
const float TRIGGER_DISTANCE_CM = 30.0;
const float RELEASE_DISTANCE_CM = 50.0;

// System States & Event Types
enum SystemState
{
    STATE_IDLE,
    STATE_AWAITING_FINGER,
    STATE_ACCESS_GRANTED,
    STATE_ACCESS_DENIED,
    STATE_UPDATE_LOG
};

enum EventType
{
    EVENT_PERSON_NEAR,
    EVENT_PERSON_LEFT,
    EVENT_FINGER_MATCHED,
    EVENT_FINGER_FAILED,
    EVENT_UPLOAD_LOG
};

// Buzzer sound patterns
enum BuzzerPattern
{
    BUZZ_WELCOME,        // Person near
    BUZZ_ACCESS_GRANTED, // Finger matched
    BUZZ_ACCESS_DENIED,  // Finger failed
    BUZZ_PHOTO_CLICK     // Photo captured
};
struct LogPayload
{
    char eventId[32];
    char reason[64]; // e.g., EVENT_FINGER_FAILED, EVENT_PERSON_NEAR
};

struct CameraEvent
{
    bool photoTaken;
    char eventId[32]; // Shared unique identifier
    unsigned long timestamp;
};
struct SystemEvent
{
    EventType type;
    union
    {
        int intValue;
        LogPayload logData;
    } payload;
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
extern QueueHandle_t xLogQueue;
extern QueueHandle_t xBuzzerQueue;

extern volatile bool isFingerprintEnabled;

// Shared Display Helper Function
void sendToLcd(const char *l1, const char *l2, bool clear = false);

#endif