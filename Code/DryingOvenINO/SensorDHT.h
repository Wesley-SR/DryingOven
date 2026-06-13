#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

struct DHTReading
{
    bool enabled;
    bool valid;
    bool timedOut;           // NEW: Track if read timed out
    float temperature;
    float humidity;
    uint8_t consecutiveTimeouts;  // NEW: Counter for timeout tracking
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
    unsigned long lastReadTime[MAX_DHT_SENSORS];  // NEW: Track read timing
    
    // NEW: Non-blocking timeout-aware read function
    bool readSensorWithTimeout(uint8_t sensorIndex, 
                               float &temperature, 
                               float &humidity);
};

#endif