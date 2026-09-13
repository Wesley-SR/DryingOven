#ifndef SENSOR_TH04S_H
#define SENSOR_TH04S_H

#include <Arduino.h>
#include <SoftwareSerial.h>


// ============================================================
// SENSOR COMMUNICATION ERROR CODES
// ============================================================

enum SensorTH04SError {
    SENSOR_TH04S_OK = 0,
    SENSOR_TH04S_ERROR_TIMEOUT,
    SENSOR_TH04S_ERROR_CRC,
    SENSOR_TH04S_ERROR_INVALID_RESPONSE,
    SENSOR_TH04S_ERROR_COMMUNICATION
};


// ============================================================
// SENSOR DATA STRUCTURE
// ============================================================

struct SensorTH04SData {
    float temperature;
    float humidity;
    bool valid;
    SensorTH04SError error;
};


// ============================================================
// SENSOR CLASS
// ============================================================

class SensorTH04S {
public:
    SensorTH04S(SoftwareSerial& serial, uint8_t deRePin, uint8_t slaveAddress);

    void begin(uint32_t baudrate);
    SensorTH04SError read();

    float getTemperature() const;
    float getHumidity() const;
    bool isValid() const;
    SensorTH04SError getLastError() const;
    uint8_t getAddress() const;

private:
    SoftwareSerial& _serial;
    uint8_t _deRePin;
    uint8_t _slaveAddress;

    float _temperature;
    float _humidity;
    bool _valid;
    SensorTH04SError _lastError;

    void enableTransmit();
    void enableReceive();

    uint16_t calculateCRC(const uint8_t* data, uint8_t length);
    bool sendReadRequest();
    bool receiveResponse();
    bool validateResponse(const uint8_t* response, uint8_t length);
    void processResponse(const uint8_t* response);
};

#endif