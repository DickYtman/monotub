#ifndef BME280Handler_h
#define BME280Handler_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class BME280Handler {
public:
    BME280Handler();
    bool begin();
    float readTemperature();
    float readHumidity();
private:
    Adafruit_BME280 bme; // I²C
};

#endif
