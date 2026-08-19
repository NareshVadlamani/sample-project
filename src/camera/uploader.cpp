#include <WiFi.h>
#include <WiFiClientSecure.h> // Required for HTTPS endpoints
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "camera_uploader.h"
#include "esp_camera.h"
#include "config.h"
#include "buzzer_sound.h"
#include "sd_offline_sync.h"

// --- ESP32-S3 CAM Pinout (Camera Bus) ---
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

static QueueHandle_t cameraQueue = NULL;
const char *UPLOAD_URL = "https://api-iot-raithunestham.onrender.com/api/upload/image";

String uploadImageToCloud(uint8_t *imageBytes, size_t length, const char *eventId)
{

  // 1. Configure secure client to bypass SSL certificate validation for HTTPS
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, UPLOAD_URL))
  {
    Serial.println("[CameraTask] Failed to connect to HTTPS endpoint");
    return "";
  }

  // 2. Define multipart boundary
  String boundary = "----ESP32Boundary12345";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  // 3. Build text field for eventId
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"eventId\"\r\n\r\n";
  head += String(eventId) + "\r\n";

  // 4. Build multipart file header
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"" + String(eventId) + ".jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  // 5. Build multipart tail
  String tail = "\r\n--" + boundary + "--\r\n";

  size_t totalLen = head.length() + length + tail.length();

  // 6. Allocate buffer in PSRAM or Heap
  uint8_t *payloadBuffer = psramFound()
                               ? (uint8_t *)ps_malloc(totalLen)
                               : (uint8_t *)malloc(totalLen);

  if (!payloadBuffer)
  {
    Serial.println("[CameraTask] Out of memory for upload buffer!");
    http.end();
    return "";
  }

  // Copy head -> JPEG bytes -> tail
  memcpy(payloadBuffer, head.c_str(), head.length());
  memcpy(payloadBuffer + head.length(), imageBytes, length);
  memcpy(payloadBuffer + head.length() + length, tail.c_str(), tail.length());

  int httpResponseCode = http.POST(payloadBuffer, totalLen);
  free(payloadBuffer); // Free memory immediately

  String responseBody = "";
  if (httpResponseCode == 200 || httpResponseCode == 201)
  {
    responseBody = http.getString();
    Serial.printf("[CameraTask] Image upload success for eventId: %s\n", eventId);
  }
  else
  {
    Serial.printf("[CameraTask] Upload failed, HTTP Code: %d\n", httpResponseCode);
  }

  http.end();
  return responseBody;
}

void captureAndUpload(CameraEvent event)
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("[Camera] Capture failed!");
    return;
  }

  Serial.printf("[Camera] Captured %u bytes. Uploading...\n", fb->len);
  triggerBuzzer(BUZZ_PHOTO_CLICK);
  String response = "";
  if (WiFi.status() == WL_CONNECTED)
  {
    response = uploadImageToCloud(fb->buf, fb->len, event.eventId);
  }
  if (response.length() == 0)
  {
    Serial.println("[Camera] Offline / upload failed. Saving image to SD card...");
    saveImageOffline(event.eventId, fb->buf, fb->len);
  }
  esp_camera_fb_return(fb); // Release frame memory back to driver
}

static void TaskCameraUploader(void *pvParameters)
{
  CameraEvent event;
  for (;;)
  {
    if (xQueueReceive(cameraQueue, &event, portMAX_DELAY) == pdTRUE)
    {
      if (!event.photoTaken)
      {
        captureAndUpload(event);
      }
    }
  }
}

bool triggerCameraUpload(const char *eventId)
{
  if (cameraQueue == NULL)
    return false;

  // Zero-initialize struct to prevent garbage memory in photoTaken
  CameraEvent event = {0};
  snprintf(event.eventId, sizeof(event.eventId), "%s", eventId);
  event.photoTaken = false;
  event.timestamp = millis();

  return xQueueSend(cameraQueue, &event, 0) == pdTRUE;
}

void initCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000; // 10MHz clock for stability
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  cameraQueue = xQueueCreate(10, sizeof(CameraEvent));

  // Raised FreeRTOS priority from 0 to 1 so the task executes reliably
  xTaskCreatePinnedToCore(
      TaskCameraUploader, "CameraCaptureTask", 20480, NULL, 1, NULL, 0);
}