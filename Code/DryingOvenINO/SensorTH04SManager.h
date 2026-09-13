#ifndef SENSOR_TH04S_MANAGER_H
#define SENSOR_TH04S_MANAGER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SensorTH04S.h"

class SensorTH04SManager
{
public:
    SensorTH04SManager(SoftwareSerial& serial, uint8_t deRePin);

    void begin(uint32_t baudrate);
    bool readAll();

    float getAverageTemperature() const;
    float getAverageHumidity() const;
    uint8_t getValidSensorCount() const;
    uint8_t getEnabledSensorCount() const;

private:
    SoftwareSerial& _serial;
    uint8_t _deRePin;

    SensorTH04S _sensor1;
    SensorTH04S _sensor2;
    SensorTH04S _sensor3;
    SensorTH04S _sensor4;

    float _averageTemperature;
    float _averageHumidity;
    uint8_t _validSensorCount;
    uint8_t _enabledSensorCount;

    bool readSensor(SensorTH04S& sensor, bool enabled, float& temperatureSum, float& humiditySum);
    bool isReadingValid(float temperature, float humidity) const;
    void printSensorStatus(const SensorTH04S& sensor, bool enabled) const;
};

#endif
