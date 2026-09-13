#ifndef CONFIG_H
#define CONFIG_H

/*************************************************
 * SERIAL CONFIGURATION
 *************************************************/
#define SERIAL_BAUDRATE 9600

/*************************************************
 * TIMING
 *************************************************/
#define MEASUREMENT_INTERVAL_MS 5000

/*************************************************
 * DISPLAY CONFIGURATION
 *************************************************/
#define LCD_I2C_ADDRESS 0x27
#define LCD_COLUMNS     16
#define LCD_ROWS        2

/*************************************************
 * DISPLAY I2C CONFIGURATION
 *************************************************/
#define LCD_TIMEOUT_MS        100    // Max 100ms per I2C operation
#define DISPLAY_ERROR_LED_PIN 12     // Optional LED to indicate display errors
#define ENABLE_DISPLAY_LED    0      // Set to 1 only if the display error LED is installed

/*************************************************
 * RS485 COMMUNICATION CONFIGURATION
 *************************************************/
#define RS485_BAUDRATE        9600

// SoftwareSerial pins used by the RS485 converter
#define RS485_RX_PIN          10
#define RS485_TX_PIN          11

// RS485 sensor configuration
#define TH04S_READ_TIMEOUT_MS 500
#define TH04S_SENSOR_1_ADDRESS 1

/*************************************************
 * RELAY / ACTUATOR PINS
 *************************************************/
#define RESISTENCE_PIN_1  6
#define RESISTENCE_PIN_2  7
#define FAN_PIN_1         4
#define FAN_HUMIDITY_PIN  5

/*************************************************
 * CONTROL PARAMETERS WITH HYSTERESIS
 *************************************************/
#define TEMP_TURN_ON_C     48.0f
#define TEMP_TURN_OFF_C    52.0f
#define TEMP_HYSTERESIS_C  (TEMP_TURN_OFF_C - TEMP_TURN_ON_C)

#define HUMIDITY_TURN_ON_PCT   90.0f
#define HUMIDITY_TURN_OFF_PCT  80.0f

/*************************************************
 * SAFETY WATCHDOG & VALIDATION
 *************************************************/
#define TEMP_SENSOR_MIN_VALID  -10.0f
#define TEMP_SENSOR_MAX_VALID  100.0f
#define HEATING_WATCHDOG_MS    2400000UL  // 40 minutes max heating time

/*************************************************
 * RELAY LOGIC (ACTIVE LOW)
 *************************************************/
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

#endif
