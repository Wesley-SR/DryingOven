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
        readings[i].temperature = 0.0f;
        readings[i].humidity = 0.0f;

        if ((sensorEnabled[i] == true) && (sensors[i] != nullptr))
        {
            float localHumidity = sensors[i]->readHumidity();
            float localTemperature = sensors[i]->readTemperature();

            if (!isnan(localHumidity) && !isnan(localTemperature))
            {
                readings[i].valid = true;
                readings[i].humidity = localHumidity;
                readings[i].temperature = localTemperature;

                humSum += localHumidity;
                tempSum += localTemperature;
                validCount++;
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