#ifndef SoilMoistureHandler_h
#define SoilMoistureHandler_h

#include <Arduino.h>

class SoilMoistureHandler {
public:
    SoilMoistureHandler(int pin);
    void begin();
    int readMoisture();

private:
    int _pin;
};

#endif
