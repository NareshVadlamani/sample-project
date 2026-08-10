#ifndef CAMERA_UPLOADER_H
#define CAMERA_UPLOADER_H

#include <Arduino.h>

// Initialize OV2640 camera hardware
void initCamera();

// Capture a frame and POST it to your server URL
void captureAndUpload();
bool triggerCameraUpload(const char *eventId);

#endif // CAMERA_UPLOADER_H