#include <SoftwareSerial.h>

#include "config.h"
#include "sensorTH04S.h"

// RS485 communication interface
SoftwareSerial RS485Serial(RS485_RX_PIN, RS485_TX_PIN);

// Sensor objects
SensorTH04S sensor1(RS485Serial, SENSOR_1_ADDRESS);
// SensorTH04S sensor2(RS485Serial, SENSOR_2_ADDRESS);
// SensorTH04S sensor3(RS485Serial, SENSOR_3_ADDRESS);

unsigned long lastSensorReadTime = 0;

void setup() {
    Serial.begin(DEBUG_BAUDRATE);

    sensor1.begin(RS485_BAUDRATE);
    // sensor2.begin(RS485_BAUDRATE);
    // sensor3.begin(RS485_BAUDRATE);

    Serial.println("Drying oven system started.");
    Serial.println("RS485 communication initialized.");
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL_MS) {
        lastSensorReadTime = currentTime;

        readSensor(sensor1);
        // readSensor(sensor2);
        // readSensor(sensor3);

        Serial.println("--------------------------------");
    }
}

void readSensor(SensorTH04S& sensor) {
    SensorTH04SError result = sensor.read();

    Serial.print("Sensor ID: ");
    Serial.println(sensor.getAddress());

    if (result == SENSOR_TH04S_OK) {
        Serial.print("Status: OK | Temperature: ");
        Serial.print(sensor.getTemperature());
        Serial.print(" C | Humidity: ");
        Serial.print(sensor.getHumidity());
        Serial.println(" %RH");
    } else {
        Serial.print("Status: ERROR | Error: ");
        Serial.println(getSensorErrorText(result));
    }
}

const char* getSensorErrorText(SensorTH04SError error) {
    switch (error) {
        case SENSOR_TH04S_OK:
            return "OK";

        case SENSOR_TH04S_ERROR_TIMEOUT:
            return "TIMEOUT";

        case SENSOR_TH04S_ERROR_CRC:
            return "CRC ERROR";

        case SENSOR_TH04S_ERROR_INVALID_RESPONSE:
            return "INVALID RESPONSE";

        case SENSOR_TH04S_ERROR_COMMUNICATION:
            return "COMMUNICATION ERROR";

        default:
            return "UNKNOWN ERROR";
    }
}