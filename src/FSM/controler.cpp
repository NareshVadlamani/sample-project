#include "fsm_controler.h"
#include "servo_motor.h"
#include "config.h"

void TaskSystemManager(void *pvParameters)
{
    SystemState currentState = STATE_IDLE;
    SystemEvent event;

    sendToLcd("System Ready", "Standby...");

    for (;;)
    {
        if (xQueueReceive(xEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {

            switch (currentState)
            {

            case STATE_IDLE:
                if (event.type == EVENT_PERSON_NEAR)
                {
                    currentState = STATE_AWAITING_FINGER;
                    sendToLcd("Welcome!", "Scan Finger...");
                    isFingerprintEnabled = true;
                    doorServo.write(180); // open finger print sensor cover
                }
                break;

            case STATE_AWAITING_FINGER:
                if (event.type == EVENT_FINGER_MATCHED)
                {
                    currentState = STATE_ACCESS_GRANTED;

                    char buf[17];
                    snprintf(buf, sizeof(buf), "User #%d", event.payload);
                    sendToLcd("Access Granted!", buf);

                    doorServo.write(90);             // Unlock door
                    vTaskDelay(pdMS_TO_TICKS(3000)); // Hold open for 3 seconds
                    doorServo.write(0);              // Relock door

                    currentState = STATE_IDLE;
                    sendToLcd("System Ready", "Standby...");
                }
                else if (event.type == EVENT_FINGER_FAILED)
                {
                    currentState = STATE_ACCESS_DENIED;
                    sendToLcd("Access Denied!", "Try Again");

                    vTaskDelay(pdMS_TO_TICKS(2000));
                    currentState = STATE_AWAITING_FINGER;
                    sendToLcd("Welcome!", "Scan Finger...");
                }
                else if (event.type == EVENT_PERSON_LEFT)
                {
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