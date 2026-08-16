#include "buzzer_sound.h"
#include "config.h"

QueueHandle_t xBuzzerQueue = NULL;

// Use LEDC Channel 1 (Channel 0 is used by Camera XCLK)
#define BUZZER_LEDC_CHANNEL 1
#define BUZZER_LEDC_RESOLUTION 8 // 8-bit resolution (0-255)

static void playTone(uint32_t freq, uint32_t durationMs)
{
    if (freq > 0)
    {
        ledcAttachPin(BUZZER_PIN, BUZZER_LEDC_CHANNEL);
        ledcWriteTone(BUZZER_LEDC_CHANNEL, freq);
        ledcWrite(BUZZER_LEDC_CHANNEL, 127); // 50% duty cycle
    }

    vTaskDelay(pdMS_TO_TICKS(durationMs));

    // Stop tone
    ledcWriteTone(BUZZER_LEDC_CHANNEL, 0);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
    ledcDetachPin(BUZZER_PIN);
}

static void TaskBuzzer(void *pvParameters)
{
    BuzzerPattern pattern;

    for (;;)
    {
        if (xQueueReceive(xBuzzerQueue, &pattern, portMAX_DELAY) == pdTRUE)
        {
            switch (pattern)
            {
            case BUZZ_WELCOME:
                playTone(2000, 60);
                vTaskDelay(pdMS_TO_TICKS(50));
                playTone(2600, 80);
                break;

            case BUZZ_ACCESS_GRANTED:
                playTone(523, 100);
                vTaskDelay(pdMS_TO_TICKS(30));
                playTone(659, 100);
                vTaskDelay(pdMS_TO_TICKS(30));
                playTone(784, 200);
                break;

            case BUZZ_ACCESS_DENIED:
                playTone(300, 250);
                vTaskDelay(pdMS_TO_TICKS(80));
                playTone(200, 350);
                break;

            case BUZZ_PHOTO_CLICK:
                playTone(1800, 30);
                vTaskDelay(pdMS_TO_TICKS(40));
                playTone(1400, 40);
                break;
            }
        }
    }
}

void triggerBuzzer(BuzzerPattern pattern)
{
    if (xBuzzerQueue != NULL)
    {
        xQueueSend(xBuzzerQueue, &pattern, 0);
    }
}

void initBuzzer()
{
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    xBuzzerQueue = xQueueCreate(5, sizeof(BuzzerPattern));

    xTaskCreatePinnedToCore(TaskBuzzer, "BUZZER_TASK", 2048, NULL, 1, NULL, 1);
    Serial.println("[Buzzer] Initialized successfully.");
}