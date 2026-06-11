#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

struct DHTReading
{
    bool enabled;
    bool valid;
    float temperature;
    float humidity;
};

class SensorDHT
{
public:
    SensorDHT();

    void begin();

    bool readAll(DHTReading readings[MAX_DHT_SENSORS],
                 float &averageTemperature,
                 float &averageHumidity,
                 uint8_t &validCount);

private:
    DHT* sensors[MAX_DHT_SENSORS];
    bool sensorEnabled[MAX_DHT_SENSORS];
};

#endif