#include <Arduino.h>
#include "lcd_helper.h"
#include "LiquidCrystal_I2C.h"

extern SemaphoreHandle_t lcdMutex;
extern LiquidCrystal_I2C lcd;
// Reference to the global LCD object declared in main.cpp

void safeLcdWrite(uint8_t col, uint8_t row, bool clearFirst, const char *format, ...)
{
    char buffer[32]; // Buffer large enough for a 16 or 20 char LCD line

    // Format the variadic string into the buffer
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Safely acquire the I2C mutex lock
    if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (clearFirst)
        {
            lcd.clear();
        }
        lcd.setCursor(col, row);
        lcd.print(buffer);

        xSemaphoreGive(lcdMutex); // Release lock
    }
}