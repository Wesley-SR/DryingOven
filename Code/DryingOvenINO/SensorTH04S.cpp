#include "sensorTH04S.h"
#include "config.h"

// Modbus function code
#define MODBUS_FUNCTION_READ_HOLDING_REGISTERS 0x03

// Sensor register configuration
// Verify these values in the CWT-TH04S datasheet.
#define TH04S_START_REGISTER 0x0000
#define TH04S_REGISTER_COUNT 2

// Expected response size:
// Address + Function + Byte Count + 4 Data Bytes + 2 CRC Bytes
#define TH04S_RESPONSE_SIZE 9

SensorTH04S::SensorTH04S(SoftwareSerial& serial, uint8_t deRePin, uint8_t slaveAddress)
    : _serial(serial),
      _deRePin(deRePin),
      _slaveAddress(slaveAddress),
      _temperature(0.0f),
      _humidity(0.0f),
      _valid(false),
      _lastError(SENSOR_TH04S_ERROR_COMMUNICATION) {
}

void SensorTH04S::begin(uint32_t baudrate) {
    pinMode(_deRePin, OUTPUT);
    enableReceive();
    _serial.begin(baudrate);
}

SensorTH04SError SensorTH04S::read() {
    _valid = false;

    while (_serial.available()) {
        _serial.read();
    }

    if (!sendReadRequest()) {
        _lastError = SENSOR_TH04S_ERROR_COMMUNICATION;
        return _lastError;
    }

    if (!receiveResponse()) {
        return _lastError;
    }

    _valid = true;
    _lastError = SENSOR_TH04S_OK;

    return _lastError;
}

void SensorTH04S::enableTransmit() {
    digitalWrite(_deRePin, HIGH);
}

void SensorTH04S::enableReceive() {
    digitalWrite(_deRePin, LOW);
}

bool SensorTH04S::sendReadRequest() {
    uint8_t request[8];

    request[0] = _slaveAddress;
    request[1] = MODBUS_FUNCTION_READ_HOLDING_REGISTERS;
    request[2] = highByte(TH04S_START_REGISTER);
    request[3] = lowByte(TH04S_START_REGISTER);
    request[4] = highByte(TH04S_REGISTER_COUNT);
    request[5] = lowByte(TH04S_REGISTER_COUNT);

    uint16_t crc = calculateCRC(request, 6);

    request[6] = lowByte(crc);
    request[7] = highByte(crc);

    enableTransmit();
    delayMicroseconds(100);

    _serial.write(request, sizeof(request));
    _serial.flush();

    delayMicroseconds(100);
    enableReceive();

    return true;
}

bool SensorTH04S::receiveResponse() {
    uint8_t response[TH04S_RESPONSE_SIZE];
    uint8_t index = 0;
    unsigned long startTime = millis();

    while (millis() - startTime < TH04S_READ_TIMEOUT_MS) {
        if (_serial.available()) {
            response[index++] = _serial.read();

            if (index >= TH04S_RESPONSE_SIZE) {
                break;
            }
        }
    }

    if (index != TH04S_RESPONSE_SIZE) {
        _lastError = SENSOR_TH04S_ERROR_TIMEOUT;
        return false;
    }

    if (response[0] != _slaveAddress) {
        _lastError = SENSOR_TH04S_ERROR_INVALID_RESPONSE;
        return false;
    }

    if (response[1] != MODBUS_FUNCTION_READ_HOLDING_REGISTERS) {
        _lastError = SENSOR_TH04S_ERROR_INVALID_RESPONSE;
        return false;
    }

    if (response[2] != 4) {
        _lastError = SENSOR_TH04S_ERROR_INVALID_RESPONSE;
        return false;
    }

    if (!validateResponse(response, TH04S_RESPONSE_SIZE)) {
        _lastError = SENSOR_TH04S_ERROR_CRC;
        return false;
    }

    processResponse(response);

    return true;
}

bool SensorTH04S::validateResponse(const uint8_t* response, uint8_t length) {
    uint16_t receivedCRC = response[length - 2] | (response[length - 1] << 8);
    uint16_t calculatedCRC = calculateCRC(response, length - 2);

    return receivedCRC == calculatedCRC;
}

uint16_t SensorTH04S::calculateCRC(const uint8_t* data, uint8_t length) {
    uint16_t crc = 0xFFFF;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

void SensorTH04S::processResponse(const uint8_t* response) {
    uint16_t temperatureRaw = (response[3] << 8) | response[4];
    uint16_t humidityRaw = (response[5] << 8) | response[6];

    _temperature = temperatureRaw / 100.0f;
    _humidity = humidityRaw / 100.0f;
}

float SensorTH04S::getTemperature() const {
    return _temperature;
}

float SensorTH04S::getHumidity() const {
    return _humidity;
}

bool SensorTH04S::isValid() const {
    return _valid;
}

SensorTH04SError SensorTH04S::getLastError() const {
    return _lastError;
}

uint8_t SensorTH04S::getAddress() const {
    return _slaveAddress;
}