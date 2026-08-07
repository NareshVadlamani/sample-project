#ifndef CAMERA_UPLOADER_H
#define CAMERA_UPLOADER_H

#include <Arduino.h>

struct CameraEvent
{
  float distance;
  bool photoTaken;
  uint32_t timestamp;
};

// Initialize OV2640 camera hardware
void initCamera();

// Capture a frame and POST it to your server URL
void captureAndUpload();
bool triggerCameraUpload();

#endif // CAMERA_UPLOADER_H