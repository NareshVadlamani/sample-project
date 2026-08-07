#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void TaskLCD(void *pvParameters)
{
    LcdMessage msg;
    for (;;)
    {
        if (xQueueReceive(xLcdQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            if (msg.clearFirst)
            {
                lcd.clear();
            }
            lcd.setCursor(0, 0);
            lcd.print(msg.line1);
            lcd.setCursor(0, 1);
            lcd.print(msg.line2);
        }
    }
}

void initializeLCD()
{
    // Initialize the LCD
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("System Ready v1.1");
    xTaskCreatePinnedToCore(TaskLCD, "LCD_Task", 3072, NULL, 2, NULL, 1);
}

void sendToLcd(const char *l1, const char *l2, bool clear)
{
    LcdMessage msg;
    snprintf(msg.line1, sizeof(msg.line1), "%-16s", l1);
    snprintf(msg.line2, sizeof(msg.line2), "%-16s", l2);
    msg.clearFirst = clear;
    xQueueSend(xLcdQueue, &msg, portMAX_DELAY);
}
