#include "fingerprint_sensor.h"
#include "config.h"

// Use HardwareSerial port 2 on ESP32-S3
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

static uint8_t nextEnrollID = 1;
static bool isEnrolling = false; // Flag to indicate if enrollment is in progress

static bool enrollFingerprint(uint8_t id)
{
  int p = -1;
  Serial.printf("Enrolling ID #%d. Please place finger on sensor...\n", id);
  delay(2000); // Wait for user to place finger
  // --- Step 1: First Scan ---

  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      Serial.println("Error taking image");
      return false;
    }
    delay(200);
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK)
  {
    Serial.println("Failed to convert image 1");
    return false;
  }

  Serial.println("Remove finger...");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  // --- Step 2: Second Scan ---
  Serial.println("Place SAME finger again...");
  delay(2000);
  p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken again");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      Serial.println("Error taking second image");
      return false;
    }
    delay(200);
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK)
  {
    Serial.println("Failed to convert image 2");
    return false;
  }

  // --- Step 3: Create and Store Model ---
  p = finger.createModel();
  delay(200);
  if (p != FINGERPRINT_OK)
  {
    Serial.println("Prints did not match! Try again.");
    return false;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.printf("SUCCESS! Fingerprint stored at ID #%d\n", id);
    return true;
  }
  else
  {
    Serial.println("Error storing model in memory");
    return false;
  }
}

int getFingerprintID()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1; // No finger detected or read error

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1; // Image conversion failed

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1; // No matching fingerprint found

  // Match found!
  Serial.printf("Found Match! ID #%d with confidence of %d\n", finger.fingerID, finger.confidence);
  return finger.fingerID;
}

void TaskFingerprint(void *pvParameters)
{
  for (;;)
  {
    if (isFingerprintEnabled)
    {
      int fingerID = getFingerprintID();
      if (fingerID >= 0)
      {
        SystemEvent ev = {EVENT_FINGER_MATCHED, {finger.fingerID}};
        xQueueSend(xEventQueue, &ev, 0);
      }
      else
      {
        SystemEvent ev = {EVENT_FINGER_FAILED, {0}};
        xQueueSend(xEventQueue, &ev, 0);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(150));
  }
}
void initFingerprint()
{
  // Start HardwareSerial2 on RX=20, TX=21 at 57600 baud (standard FP baud rate)
  mySerial.begin(57600, SERIAL_8N1, FP_RX_PIN, FP_TX_PIN);
  pinMode(ENROLL_BTN_PIN, INPUT_PULLUP); // Set Enroll Button Pin as Input with Pull-up
  delay(100);
  if (finger.verifyPassword())
  {
    finger.setSecurityLevel(1);
    finger.getTemplateCount();
    nextEnrollID = finger.templateCount + 1;
    xTaskCreatePinnedToCore(
        TaskFingerprint, "FingerprintTask", 4096, NULL, 4, NULL, 1);
    return;
  }
  else
  {
    Serial.println("Fingerprint sensor NOT found! Check wiring.");
    return;
  }
}
