#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// RS485 communication
#define RS485_RX_PIN 10
#define RS485_TX_PIN 11
#define RS485_BAUDRATE 4800

// Sensor configuration
#define NUMBER_OF_SENSORS 1
#define SENSOR_READ_INTERVAL_MS 2000
#define SENSOR_TIMEOUT_MS 500

// Modbus sensor addresses
#define SENSOR_1_ADDRESS 1
#define SENSOR_2_ADDRESS 2
#define SENSOR_3_ADDRESS 3

// Serial Monitor
#define DEBUG_BAUDRATE 9600

#endif