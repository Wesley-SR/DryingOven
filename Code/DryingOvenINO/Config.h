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
#define DISPLAY_ERROR_LED_PIN 12     // LED to indicate display errors (optional)
#define ENABLE_DISPLAY_LED    0      // Set to 0 if no LED available

/*************************************************
 * RS485 COMMUNICATION CONFIGURATION
 *************************************************/
#define RS485_BAUDRATE       9600
#define RS485_SERIAL_CONFIG  SERIAL_8N1

// RS485 serial pins
#define RS485_RX_PIN         10
#define RS485_TX_PIN         11

// RS485 driver enable / receiver enable pin
#define RS485_DE_RE_PIN      4

#define MAX_TH04S_SENSORS    4
#define TH04S_READ_TIMEOUT_MS 500 // Timeout for RS485 read operations

// Enable only the sensors installed in the system
#define EN_TH04S_SENSOR_1    1
#define EN_TH04S_SENSOR_2    1
#define EN_TH04S_SENSOR_3    0
#define EN_TH04S_SENSOR_4    0

// Modbus slave addresses
#define TH04S_SENSOR_1_ADDRESS 1
#define TH04S_SENSOR_2_ADDRESS 2
#define TH04S_SENSOR_3_ADDRESS 3
#define TH04S_SENSOR_4_ADDRESS 4

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
#define TEMP_TURN_ON_C     48.0f   // 48 Start heating
#define TEMP_TURN_OFF_C    52.0f   // 56 Stop heating
#define TEMP_HYSTERESIS_C  (TEMP_TURN_OFF_C - TEMP_TURN_ON_C)

#define HUMIDITY_TURN_ON_PCT   90.0f   // Start dehumidifying below this
#define HUMIDITY_TURN_OFF_PCT  80.0f   // Stop dehumidifying above this

/*************************************************
 * SAFETY WATCHDOG & VALIDATION
 *************************************************/
#define TEMP_SENSOR_MIN_VALID  -10.0f   // Minimum valid temperature
#define TEMP_SENSOR_MAX_VALID  +100.0f  // Maximum valid temperature
#define HEATING_WATCHDOG_MS    2400000  // 40 minutes max heating

/*************************************************
 * RELAY LOGIC (ACTIVE LOW)
 *************************************************/
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

#endif