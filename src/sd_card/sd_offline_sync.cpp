#include "sd_offline_sync.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

static SPIClass sdSPI(FSPI);
static const char *LOG_SERVICE_URL = "https://api-iot-raithunestham.onrender.com/api/usersEntry/add";
static const char *IMG_SERVICE_URL = "https://api-iot-raithunestham.onrender.com/api/usersEntry/uploadImage";

bool initSDCard()
{
    sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, sdSPI, 40000000))
    {
        Serial.println("[SD] Mount Failed!");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("[SD] No SD card attached.");
        return false;
    }

    Serial.println("[SD] MicroSD Card Initialized.");
    return true;
}

bool saveLogOffline(const char *eventId, const char *reason)
{
    char path[64];
    snprintf(path, sizeof(path), "/log_%s.json", eventId);

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.printf("[SD] Failed to create %s\n", path);
        return false;
    }

    JsonDocument doc;
    doc["eventId"] = eventId;
    doc["reason"] = reason;

    serializeJson(doc, file);
    file.close();
    Serial.printf("[SD] Log saved offline: %s\n", path);
    return true;
}

bool saveImageOffline(const char *eventId, const uint8_t *buf, size_t len)
{
    char path[64];
    snprintf(path, sizeof(path), "/img_%s.jpg", eventId);

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.printf("[SD] Failed to create %s\n", path);
        return false;
    }

    file.write(buf, len);
    file.close();
    Serial.printf("[SD] Image saved offline (%u bytes): %s\n", len, path);
    return true;
}

// Background Sync Task
static void TaskOfflineSync(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(15000)); // Scan every 15 seconds

        if (WiFi.status() != WL_CONNECTED)
            continue;

        File root = SD.open("/");
        if (!root || !root.isDirectory())
            continue;

        File file = root.openNextFile();
        while (file)
        {
            String fileName = String(file.name());

            // 1. Sync Pending Logs
            if (fileName.startsWith("log_") && fileName.endsWith(".json"))
            {
                String fullPath = "/" + fileName;
                size_t size = file.size();
                std::unique_ptr<char[]> buf(new char[size + 1]);
                file.readBytes(buf.get(), size);
                buf[size] = '\0';
                file.close();

                WiFiClientSecure client;
                client.setInsecure();
                HTTPClient http;
                http.setTimeout(30000);

                if (http.begin(client, LOG_SERVICE_URL))
                {
                    http.addHeader("Content-Type", "application/json");
                    int httpCode = http.POST((uint8_t *)buf.get(), size);

                    if (httpCode == 200 || httpCode == 201)
                    {
                        Serial.printf("[SD Sync] Log synced successfully: %s\n", fullPath.c_str());
                        SD.remove(fullPath.c_str());
                    }
                    http.end();
                }
            }
            // 2. Sync Pending Images
            else if (fileName.startsWith("img_") && fileName.endsWith(".jpg"))
            {
                String fullPath = "/" + fileName;
                // Extract eventId from "img_<eventId>.jpg"
                String eventId = fileName.substring(4, fileName.length() - 4);

                size_t imgSize = file.size();
                uint8_t *imgBuf = (uint8_t *)malloc(imgSize);

                if (imgBuf)
                {
                    file.read(imgBuf, imgSize);
                    file.close();

                    WiFiClientSecure client;
                    client.setInsecure();
                    HTTPClient http;
                    http.setTimeout(45000);

                    String uploadUrl = String(IMG_SERVICE_URL) + "?eventId=" + eventId;
                    if (http.begin(client, uploadUrl))
                    {
                        http.addHeader("Content-Type", "image/jpeg");
                        int httpCode = http.POST(imgBuf, imgSize);

                        if (httpCode == 200 || httpCode == 201)
                        {
                            Serial.printf("[SD Sync] Image synced successfully: %s\n", fullPath.c_str());
                            SD.remove(fullPath.c_str());
                        }
                        http.end();
                    }
                    free(imgBuf);
                }
            }

            file = root.openNextFile();
        }
    }
}

void initOfflineSyncTask()
{
    // Task needs 12KB stack for SSL processing
    xTaskCreatePinnedToCore(TaskOfflineSync, "SD_SYNC_TASK", 12288, NULL, 1, NULL, 0);
}