#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "network_add_logs.h"
#include "config.h"
#include "wifi_connect.h"

const char *SERVICE_URL = "https://api-iot-raithunestham.onrender.com/api/usersEntry/add";

void TaskNetworkLogger(void *pvParameters)
{
    SystemEvent event;

    for (;;)
    {
        if (xQueueReceive(xLogQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                WiFiClientSecure client;
                client.setInsecure(); // Bypass SSL certificate verification

                HTTPClient http;
                if (!http.begin(client, SERVICE_URL))
                {
                    Serial.println("[NetworkLogger] Failed to initialize SSL connection!");
                    continue;
                }

                http.addHeader("Content-Type", "application/json");

                JsonDocument logDoc;
                logDoc["eventId"] = event.payload.logData.eventId;
                logDoc["reason"] = event.payload.logData.reason;

                String requestBody;
                serializeJson(logDoc, requestBody);

                int httpCode = http.POST(requestBody);
                if (httpCode == 200 || httpCode == 201)
                {
                    Serial.printf("[NetworkLogger] Log uploaded successfully! eventId: %s\n",
                                  event.payload.logData.eventId);
                }
                else
                {
                    Serial.printf("[NetworkLogger] Logging server error code: %d\n", httpCode);
                }

                http.end();
            }
            else
            {
                Serial.println("[NetworkLogger] Wi-Fi disconnected. Log skipped.");
            }
        }
    }
}

// FIX 3: Properly match return type declared in header file
bool triggerAddLog(const char *eventId, const char *reason)
{
    if (xLogQueue == NULL)
        return false;

    SystemEvent logEv;
    logEv.type = EVENT_UPLOAD_LOG;

    snprintf(logEv.payload.logData.eventId, sizeof(logEv.payload.logData.eventId), "%s", eventId);
    snprintf(logEv.payload.logData.reason, sizeof(logEv.payload.logData.reason), "%s", reason);

    return (xQueueSend(xLogQueue, &logEv, 0) == pdTRUE);
}