#include <Arduino.h>
#include <sys/time.h>

void initTime()
{
    // Sync time with NTP server (0 offset for UTC)
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("[Time] Syncing NTP time");
    time_t now = 0;
    int timeout = 0;
    while (now < 8 * 3600 && timeout < 20)
    { // Wait up to 2 seconds for NTP sync
        delay(100);
        time(&now);
        timeout++;
    }
    Serial.println("\n[Time] NTP Synchronized!");
}

void generateEventId(char *buffer, size_t len)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Calculate total milliseconds since Unix Epoch
    uint64_t timestampMs = ((uint64_t)tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    // Formats as "1786194964123"
    snprintf(buffer, len, "%llu", timestampMs);
}