#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "network_add_logs.h"
#include "config.h"
#include "wifi_connect.h"

const char *SERVICE_URL = "https://api-iot-raithunestham.onrender.com/api/users/addUser";

void TaskNetworkLogger(void *pvParameters)
{
    LogPayload payload;

    for (;;)
    {
        // Wait indefinitely for a log event to enter the pipeline
        if (xQueueReceive(xLogQueue, &payload, portMAX_DELAY) == pdTRUE)
        {

            if (WiFi.status() == WL_CONNECTED)
            {
                HTTPClient http;
                http.begin(SERVICE_URL);
                http.addHeader("Content-Type", "application/json");

                // Build log payload expected by backend server
                JsonDocument logDoc;
                logDoc["imageUrl"] = payload.imageUrl;
                logDoc["name"] = payload.name;

                String requestBody;
                serializeJson(logDoc, requestBody);

                int httpCode = http.POST(requestBody);
                if (httpCode != 200 && httpCode != 201)
                {
                    // Optional: Re-queue payload or save to SD card offline buffer
                    Serial.printf("Logging server returned error: %d\n", httpCode);
                }
                http.end();
            }
        }
    }
}