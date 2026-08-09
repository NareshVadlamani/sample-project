#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "network_add_logs.h"
#include "config.h"
#include "wifi_connect.h"

const char *SERVICE_URL = "https://api-iot-raithunestham.onrender.com/api/usersEntry/add";

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
                logDoc["eventId"] = payload.eventId;
                logDoc["reason"] = payload.reason;

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

void triggerAddLog(const char *eventId, const char *reason)
{
    if (xLogQueue == NULL)
        return;

    SystemEvent logEv;
    logEv.type = EVENT_UPLOAD_LOG;

    snprintf(logEv.payload.logData.eventId, sizeof(logEv.payload.logData.eventId), "%s", eventId);
    snprintf(logEv.payload.logData.reason, sizeof(logEv.payload.logData.reason), "%s", reason);

    xQueueSend(xLogQueue, &logEv, 0) == pdTRUE;
    return;
}