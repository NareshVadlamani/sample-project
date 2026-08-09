#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "camera_uploader.h"
#include "esp_camera.h"
#include "config.h"

// Reference global LCD object declared in main.cpp

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
// Helper to stream image bytes to custom cloud uploader

String uploadImageToCloud(uint8_t *imageBytes, size_t length)
{
  if (WiFi.status() != WL_CONNECTED)
    return "";

  HTTPClient http;
  http.begin(UPLOAD_URL);

  // 1. Define multipart boundary
  String boundary = "----ESP32Boundary12345";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  // 2. Build multipart head (matches curl -F "image=@...")
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"photo.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  // 3. Build multipart tail
  String tail = "\r\n--" + boundary + "--\r\n";

  // 4. Calculate total payload length
  size_t totalLen = head.length() + length + tail.length();

  // 5. Allocate buffer & assemble payload
  uint8_t *payloadBuffer = psramFound()
                               ? (uint8_t *)ps_malloc(totalLen)
                               : (uint8_t *)malloc(totalLen);

  if (!payloadBuffer)
  {
    Serial.println("[CameraTask] Out of memory for upload buffer!");
    http.end();
    return "";
  }

  // Copy head -> JPEG binary bytes -> tail
  memcpy(payloadBuffer, head.c_str(), head.length());
  memcpy(payloadBuffer + head.length(), imageBytes, length);
  memcpy(payloadBuffer + head.length() + length, tail.c_str(), tail.length());

  // 6. Send HTTP POST
  int httpResponseCode = http.POST(payloadBuffer, totalLen);
  free(payloadBuffer); // Free memory buffer immediately

  String responseBody = "";
  if (httpResponseCode == 200 || httpResponseCode == 201)
  {
    responseBody = http.getString();
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
  String responseJson = uploadImageToCloud(fb->buf, fb->len);
  esp_camera_fb_return(fb); // Release frame memory back to driver immediately
  event.photoTaken = true;

  if (responseJson.length() > 0)
  {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, responseJson);

    // 1. Check deserialization success AND API success status
    if (!err && doc["success"].as<bool>())
    {
      const char *extractedUrl = doc["data"]["public_id"];

      // 2. Safeguard against nullptr before copying string
      if (extractedUrl != nullptr)
      {
        SystemEvent logEvent;
        logEvent.type = EVENT_UPLOAD_LOG;

        // Populate logData inside the union
        snprintf(logEvent.payload.logData.imageUrl, sizeof(logEvent.payload.logData.imageUrl), "%s", extractedUrl);
        snprintf(logEvent.payload.logData.name, sizeof(logEvent.payload.logData.name), "%s", "Naresh");
        // logEvent.payload.logData.userId = 101;
        // logEvent.payload.logData.logReason = EVENT_FINGER_MATCHED;

        // 2. Dispatch event to xEventQueue
        xQueueSend(xEventQueue, &logEvent, 0);
      }
    }
  }
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

bool triggerCameraUpload()
{
  if (cameraQueue == NULL)
    return false;
  CameraEvent event = {0.0f, false, millis()};
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // --- ADD / UPDATE THIS LINE HERE ---
  config.xclk_freq_hz = 10000000; // Lowered to 10MHz to fix error code -1

  // Quality settings based on PSRAM availability
  if (psramFound())
  {
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 12;          // Lower = better quality (10-63)
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  // Camera initialization
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  cameraQueue = xQueueCreate(5, sizeof(CameraEvent));

  xTaskCreatePinnedToCore(
      TaskCameraUploader, "CameraCaptureTask", 8192, NULL, 0, NULL, 1);
}
