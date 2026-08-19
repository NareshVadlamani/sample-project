#ifndef SD_OFFLINE_SYNC_H
#define SD_OFFLINE_SYNC_H

#include <Arduino.h>

bool initSDCard();
bool saveLogOffline(const char *eventId, const char *reason);
bool saveImageOffline(const char *eventId, const uint8_t *buf, size_t len);
void initOfflineSyncTask();

#endif