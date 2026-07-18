#include "SensorDHT.h"

SensorDHT::SensorDHT()
{
    for (int i = 0; i < MAX_DHT_SENSORS; i++)
    {
        sensors[i] = nullptr;
        sensorEnabled[i] = false;
    }

#if EN_DHT_SENSOR_1
    sensors[0] = new DHT(DHT_PIN_1, DHT_TYPE);
    sensorEnabled[0] = true;
#endif

#if EN_DHT_SENSOR_2
    sensors[1] = new DHT(DHT_PIN_2, DHT_TYPE);
    sensorEnabled[1] = true;
#endif

#if EN_DHT_SENSOR_3
    sensors[2] = new DHT(DHT_PIN_3, DHT_TYPE);
    sensorEnabled[2] = true;
#endif

#if EN_DHT_SENSOR_4
    sensors[3] = new DHT(DHT_PIN_4, DHT_TYPE);
    sensorEnabled[3] = true;
#endif
}

void SensorDHT::begin()
{
    for (int i = 0; i < MAX_DHT_SENSORS; i++)
    {
        if ((sensorEnabled[i] == true) && (sensors[i] != nullptr))
        {
            sensors[i]->begin();
        }
    }
}

bool SensorDHT::readSensorWithTimeout(uint8_t sensorIndex, 
                                       float &temperature, 
                                       float &humidity)
{
    if ((sensorEnabled[sensorIndex] == false) || 
        (sensors[sensorIndex] == nullptr))
    {
        return false;
    }

    unsigned long startTime = millis();
    
    // Attempt non-blocking read
    float localTemp = sensors[sensorIndex]->readTemperature();
    float localHum = sensors[sensorIndex]->readHumidity();
    
    unsigned long elapsedTime = millis() - startTime;
    
    // Check if read took too long
    if (elapsedTime > DHT_READ_TIMEOUT_MS)
    {
        Serial.print(F("Sensor "));
        Serial.print(sensorIndex + 1);
        Serial.print(F(" READ TIMEOUT: "));
        Serial.print(elapsedTime);
        Serial.println(F(" ms"));
        return false;
    }
    
    // Check for NaN (sensor error)
    if (isnan(localTemp) || isnan(localHum))
    {
        return false;
    }
    
    temperature = localTemp;
    humidity = localHum;
    return true;
}

bool SensorDHT::readAll(DHTReading readings[MAX_DHT_SENSORS],
                        float &averageTemperature,
                        float &averageHumidity,
                        uint8_t &validCount)
{
    float tempSum = 0.0f;
    float humSum = 0.0f;
    validCount = 0;

    for (int i = 0; i < MAX_DHT_SENSORS; i++)
    {
        readings[i].enabled = sensorEnabled[i];
        readings[i].valid = false;
        readings[i].timedOut = false;  // NEW
        readings[i].temperature = 0.0f;
        readings[i].humidity = 0.0f;
        readings[i].consecutiveTimeouts = 0;  // NEW

        if ((sensorEnabled[i] == true) && (sensors[i] != nullptr))
        {
            float localHumidity = 0.0f;
            float localTemperature = 0.0f;
            
            // NEW: Use timeout-aware read
            if (readSensorWithTimeout(i, localTemperature, localHumidity))
            {
                readings[i].valid = true;
                readings[i].humidity = localHumidity;
                readings[i].temperature = localTemperature;
                readings[i].consecutiveTimeouts = 0;  // Reset timeout counter on success
                
                humSum += localHumidity;
                tempSum += localTemperature;
                validCount++;
            }
            else
            {
                // NEW: Track consecutive timeouts
                readings[i].timedOut = true;
                readings[i].consecutiveTimeouts++;
                
                if (readings[i].consecutiveTimeouts >= DHT_STALL_THRESHOLD)
                {
                    Serial.print(F("Sensor "));
                    Serial.print(i + 1);
                    Serial.println(F(" marked as UNRELIABLE (too many timeouts)"));
                }
            }
        }
    }

    if (validCount == 0)
    {
        averageTemperature = 0.0f;
        averageHumidity = 0.0f;
        return false;
    }

    averageTemperature = tempSum / validCount;
    averageHumidity = humSum / validCount;
    
    return true;
}