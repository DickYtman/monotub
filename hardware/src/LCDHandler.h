#ifndef LCDHandler_h
#define LCDHandler_h

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LCDHandler {
public:
  LCDHandler();
  void init();
  void print(const String& message, uint8_t row);
  void clear();  

private:
  LiquidCrystal_I2C lcd;
};

#endif
