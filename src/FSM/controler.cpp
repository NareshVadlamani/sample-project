#include "fsm_controler.h"
#include "servo_motor.h"
#include "config.h"
#include "fingerprint_sensor.h"
#include "ultrasonic_sensor.h"
#include "camera_uploader.h"
#include "helper_timer.h"
#include "network_add_logs.h"

void TaskSystemManager(void *pvParameters)
{
    SystemState currentState = STATE_IDLE;
    SystemEvent event;
    uint8_t retryCount = 0;

    sendToLcd("System Ready", "Standby...");

    for (;;)
    {
        if (xQueueReceive(xEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {

            // --- 2. FINITE STATE MACHINE ---
            switch (currentState)
            {

            case STATE_IDLE:
                if (event.type == EVENT_PERSON_NEAR)
                {
                    retryCount = 0; // Reset retry count when a new person is detected
                    sendToLcd("Welcome!", "Scan Finger...");
                    isFingerprintEnabled = true;
                    vTaskDelay(pdMS_TO_TICKS(500)); // Allow time for the fingerprint sensor to initialize
                    doorServo.write(180);           // open finger print sensor cover
                    currentState = STATE_AWAITING_FINGER;
                }
                break;

            case STATE_AWAITING_FINGER:
                if (event.type == EVENT_FINGER_MATCHED)
                {
                    currentState = STATE_ACCESS_GRANTED;

                    char eventId[32];
                    generateEventId(eventId, sizeof(eventId));

                    char buf[17];
                    snprintf(buf, sizeof(buf), "User #%d", event.payload.intValue);
                    sendToLcd("Access Granted!", buf);
                    triggerCameraUpload(eventId); // Capture and upload photo
                    triggerAddLog(eventId, "FINGERPRINT_MATCHED");

                    doorServo.write(90);             // Unlock door
                    vTaskDelay(pdMS_TO_TICKS(3000)); // Hold open for 3 seconds
                    doorServo.write(0);              // Relock door

                    resetUltrasonicPresence();
                    currentState = STATE_IDLE;
                    sendToLcd("System Ready", "Standby...");
                }
                else if (event.type == EVENT_FINGER_FAILED)
                {
                    retryCount++;
                    if (retryCount >= 3)
                    {
                        retryCount = 0; // Reset retry count after reaching the limit
                        currentState = STATE_ACCESS_DENIED;

                        char eventId[32];
                        generateEventId(eventId, sizeof(eventId));

                        sendToLcd("Access Denied!", "Try Again");
                        triggerCameraUpload(eventId); // Capture and upload photo of the person
                        triggerAddLog(eventId, "FINGERPRINT_FAILED");
                        vTaskDelay(pdMS_TO_TICKS(2000)); // Display message for 2 seconds
                        resetUltrasonicPresence();       // Clear any pending events
                        currentState = STATE_IDLE;
                        sendToLcd("System Ready", "Standby...");
                    }
                    else
                    {
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        currentState = STATE_AWAITING_FINGER;
                        sendToLcd("Welcome!", "Scan Finger...");
                    }
                }
                else if (event.type == EVENT_PERSON_LEFT)
                {
                    doorServo.write(0); // Close sensor cover
                    resetUltrasonicPresence();
                    currentState = STATE_IDLE;
                    sendToLcd("System Ready", "Standby...");
                }
                break;

            default:
                break;
            }
        }
    }
}