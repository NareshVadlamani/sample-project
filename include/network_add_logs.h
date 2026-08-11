#ifndef NETWORK_LOGGER_TASK_H
#define NETWORK_LOGGER_TASK_H

#include <Arduino.h>

void TaskNetworkLogger(void *pvParameters);
bool triggerAddLog(const char *eventId, const char *reason);

#endif