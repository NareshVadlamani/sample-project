#include "fingerprint_sensor.h"
#include <Adafruit_Fingerprint.h>
#include "lcd_helper.h"

// Use HardwareSerial port 2 on ESP32-S3
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

static uint8_t nextEnrollID = 1;
static bool isEnrolling = false; // Flag to indicate if enrollment is in progress

int getFingerprintID()
{
  safeLcdWrite(0, 0, true, "Scanning Finger...");
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

static bool enrollFingerprint(uint8_t id)
{
  int p = -1;
  Serial.printf("Enrolling ID #%d. Please place finger on sensor...\n", id);
  safeLcdWrite(0, 1, false, "Place Finger...");
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
  safeLcdWrite(0, 1, false, "Remove Finger...");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  // --- Step 2: Second Scan ---
  Serial.println("Place SAME finger again...");
  safeLcdWrite(0, 1, false, "Place SAME finger...");
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
  safeLcdWrite(0, 1, false, "Creating Model...");
  delay(200);
  if (p != FINGERPRINT_OK)
  {
    Serial.println("Prints did not match! Try again.");
    safeLcdWrite(0, 1, false, "Prints did not match!");
    return false;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.printf("SUCCESS! Fingerprint stored at ID #%d\n", id);
    safeLcdWrite(0, 1, false, "Stored at ID #%d", id);
    return true;
  }
  else
  {
    Serial.println("Error storing model in memory");
    return false;
  }
}

static void TaskFingerprint(void *pvParameters)

{
  for (;;)
  {
    // 1. Check if Enroll Button is Pressed (Active LOW)
    if (digitalRead(ENROLL_BTN_PIN) == HIGH && !isEnrolling)
    {
      isEnrolling = true;
      vTaskDelay(pdMS_TO_TICKS(200)); // Simple button debounce delay

      safeLcdWrite(0, 0, true, "user1 ID #%d", nextEnrollID);

      if (enrollFingerprint(nextEnrollID))
      {
        safeLcdWrite(0, 1, false, "Enroll Success!");
        nextEnrollID++; // Increment for next user
      }
      else
      {
        safeLcdWrite(0, 1, false, "Enroll Failed! ");
      }

      vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 2 sec
      isEnrolling = false;
    }

    // 2. Continuous Scan for Access (Only when not enrolling)
    if (!isEnrolling)
    {
      int id = getFingerprintID();
      if (id > 0)
      {
        safeLcdWrite(0, 0, true, "User #%d Matched! ", id);
        // Trigger door lock, servo, or camera upload here!
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Yield 100ms CPU time
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
    Serial.println("Fingerprint sensor found!");
    safeLcdWrite(0, 1, false, "Fingerprint Ready! ");
    finger.getTemplateCount();
    nextEnrollID = finger.templateCount + 1;
    xTaskCreatePinnedToCore(
        TaskFingerprint, "FingerprintTask", 4096, NULL, 0, NULL, 1);
    return;
  }
  else
  {
    Serial.println("Fingerprint sensor NOT found! Check wiring.");
    safeLcdWrite(0, 1, false, "Fingerprint Error! ");
    return;
  }
}
