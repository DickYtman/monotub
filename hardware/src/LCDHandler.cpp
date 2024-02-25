#include "LCDHandler.h"

LCDHandler::LCDHandler() : lcd(0x27, 16, 2) {
    // Constructor, the lcd object is initialized here with the given I2C address, columns and rows.
}

void LCDHandler::init() {
    // Initialize the LCD
    lcd.init();  // Modified this line
    lcd.backlight();
    // lcd.print("LCD Initialized!");
}

void LCDHandler::print(const String& message, uint8_t row) {
    lcd.setCursor(0, row);
    lcd.print(message);
}

void LCDHandler::clear() {  // Define this method
    lcd.clear();
}