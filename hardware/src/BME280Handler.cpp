#include "BME280Handler.h"

BME280Handler::BME280Handler() {
    // Constructor
}

bool BME280Handler::begin() {
    if (!bme.begin(0x76)) { // check if the sensor is connected properly
        return false;
    }
    return true;
}

float BME280Handler::readTemperature() {
    return bme.readTemperature();
}

float BME280Handler::readHumidity() {
    return bme.readHumidity();
}
