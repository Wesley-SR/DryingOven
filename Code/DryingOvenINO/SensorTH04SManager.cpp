#include "SensorTH04SManager.h"
#include "config.h"

SensorTH04SManager::SensorTH04SManager(SoftwareSerial& serial, uint8_t deRePin)
    : _serial(serial),
      _deRePin(deRePin),
      _sensor1(serial, deRePin, TH04S_SENSOR_1_ADDRESS),
      _sensor2(serial, deRePin, TH04S_SENSOR_2_ADDRESS),
      _sensor3(serial, deRePin, TH04S_SENSOR_3_ADDRESS),
      _sensor4(serial, deRePin, TH04S_SENSOR_4_ADDRESS),
      _averageTemperature(0.0f),
      _averageHumidity(0.0f),
      _validSensorCount(0),
      _enabledSensorCount(0)
{
}

void SensorTH04SManager::begin(uint32_t baudrate)
{
    _sensor1.begin(baudrate);
}

bool SensorTH04SManager::readAll()
{
    float temperatureSum = 0.0f;
    float humiditySum = 0.0f;

    _validSensorCount = 0;
    _enabledSensorCount = 0;

    readSensor(_sensor1, EN_TH04S_SENSOR_1, temperatureSum, humiditySum);
    readSensor(_sensor2, EN_TH04S_SENSOR_2, temperatureSum, humiditySum);
    readSensor(_sensor3, EN_TH04S_SENSOR_3, temperatureSum, humiditySum);
    readSensor(_sensor4, EN_TH04S_SENSOR_4, temperatureSum, humiditySum);

    if (_validSensorCount == 0)
    {
        _averageTemperature = 0.0f;
        _averageHumidity = 0.0f;
        return false;
    }

    _averageTemperature = temperatureSum / _validSensorCount;
    _averageHumidity = humiditySum / _validSensorCount;

    return true;
}

float SensorTH04SManager::getAverageTemperature() const
{
    return _averageTemperature;
}

float SensorTH04SManager::getAverageHumidity() const
{
    return _averageHumidity;
}

uint8_t SensorTH04SManager::getValidSensorCount() const
{
    return _validSensorCount;
}

uint8_t SensorTH04SManager::getEnabledSensorCount() const
{
    return _enabledSensorCount;
}

bool SensorTH04SManager::readSensor(SensorTH04S& sensor, bool enabled, float& temperatureSum, float& humiditySum)
{
    if (!enabled)
    {
        return false;
    }

    _enabledSensorCount++;

    SensorTH04SError result = sensor.read();
    printSensorStatus(sensor, true);

    if (result != SENSOR_TH04S_OK || !sensor.isValid())
    {
        return false;
    }

    float temperature = sensor.getTemperature();
    float humidity = sensor.getHumidity();

    if (!isReadingValid(temperature, humidity))
    {
        return false;
    }

    temperatureSum += temperature;
    humiditySum += humidity;
    _validSensorCount++;

    return true;
}

bool SensorTH04SManager::isReadingValid(float temperature, float humidity) const
{
    if (temperature < TEMP_SENSOR_MIN_VALID || temperature > TEMP_SENSOR_MAX_VALID)
    {
        return false;
    }

    if (humidity < 0.0f || humidity > 100.0f)
    {
        return false;
    }

    return true;
}

void SensorTH04SManager::printSensorStatus(const SensorTH04S& sensor, bool enabled) const
{
    if (!enabled)
    {
        return;
    }

    Serial.print(F("TH04S address "));
    Serial.print(sensor.getAddress());
    Serial.print(F(": "));

    if (sensor.isValid())
    {
        Serial.print(F("T="));
        Serial.print(sensor.getTemperature());
        Serial.print(F(" C | H="));
        Serial.print(sensor.getHumidity());
        Serial.println(F(" %"));
    }
    else
    {
        Serial.print(F("ERROR "));
        Serial.println(sensor.getLastError());
    }
}
