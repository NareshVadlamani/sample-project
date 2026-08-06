#include "LiquidCrystal_I2C.h"
#include <Wire.h>

const uint8_t SDA_PIN = 41; // I2C SDA pin for LCD
const uint8_t SCL_PIN = 42; // I2C SCL pin

void initializeLCD() {
    // Initialize the LCD
    Wire.begin(SDA_PIN, SCL_PIN);
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    lcd.init();
    lcd.backlight(); 
    lcd.setCursor(0, 0);
}