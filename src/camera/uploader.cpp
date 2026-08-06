#include "camera_uploader.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "LiquidCrystal_I2C.h"

// Reference global LCD object declared in main.cpp
extern LiquidCrystal_I2C lcd;

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

bool initCamera()
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
    lcd.setCursor(0, 0);
    lcd.print(err);
    return false;
  }

  cameraQueue = xQueueCreate(5, sizeof(CameraEvent));

  return true;
}

bool captureAndUpload(const char *serverUrl)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Cannot upload: WiFi disconnected");
    return false;
  }

  // 1. Capture image frame from camera
  camera_fb_t *fb = esp_camera_fb_get();
  lcd.setCursor(0, 1);
  if (!fb)
  {
    lcd.print("Camera capture failed!");
    Serial.println("Camera capture failed!");
    return false;
  }

  Serial.printf("Captured image size: %u bytes. Uploading...\n", fb->len);
  lcd.print("Uploading photo...");
  // 2. Prepare HTTP POST request
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  // 3. Send raw JPEG buffer via POST
  int httpResponseCode = http.POST(fb->buf, fb->len);

  bool success = false;
  if (httpResponseCode > 0)
  {
    Serial.printf("Upload successful! HTTP Response code: %d\n", httpResponseCode);
    success = true;
  }
  else
  {
    Serial.printf("Upload failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  // 4. Clean up resources
  http.end();
  esp_camera_fb_return(fb); // Release frame memory back to camera driver

  return success;
}
