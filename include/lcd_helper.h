#ifndef LCD_HELPER_H
#define LCD_HELPER_H
#include <stdint.h>
#include <Arduino.h>
void safeLcdWrite(uint8_t col, uint8_t row, bool clearFirst, const char *format, ...);

#endif // LCD_HELPER_H
