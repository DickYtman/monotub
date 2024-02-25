#include "SoilMoistureHandler.h"

SoilMoistureHandler::SoilMoistureHandler(int pin) : _pin(pin) {
}

void SoilMoistureHandler::begin() {
    pinMode(_pin, INPUT);
}

int SoilMoistureHandler::readMoisture() {
    return analogRead(_pin);
}
