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
 * DHT SENSOR CONFIGURATION
 *************************************************/
#define DHT_TYPE  DHT22

/* Enable sensors */
#define EN_DHT_SENSOR_1 1
#define EN_DHT_SENSOR_2 1
#define EN_DHT_SENSOR_3 0
#define EN_DHT_SENSOR_4 0

#define MAX_DHT_SENSORS 2

/* Sensor pins */
#define DHT_PIN_1 9
#define DHT_PIN_2 8
#define DHT_PIN_3 10
#define DHT_READ_TIMEOUT_MS   2500
#define DHT_STALL_THRESHOLD   3


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

#define MAX_TH04S_SENSORS    8 // Maximum number of sensors supported

#define TH04S_READ_TIMEOUT_MS  500 // Timeout for RS485 read operations

#define TH04S_SENSOR_1_ADDRESS     1

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
#define TEMP_SENSOR_MIN_VALID  -10.0f   // DHT22 valid range
#define TEMP_SENSOR_MAX_VALID  +100.0f   // DHT22 valid range
#define HEATING_WATCHDOG_MS    2400000   // 40 minutes max heating

/*************************************************
 * STATUS LED INDICATORS
 *************************************************/
#define STATUS_LED_GREEN_PIN   A0   // System OK, operation normal
#define STATUS_LED_RED_PIN     A1   // Error detected (sensor failure, etc)
#define HEATING_LED_PIN        A2   // Heating active (resistance ON)
#define SENSOR_LED_PIN         A3   // Sensor OK indicator

#define ENABLE_STATUS_LEDS     0    // Set to 0 to disable all LEDs

// LED Blink patterns for diagnostics
#define LED_BLINK_FAST_MS      200  // Fast blink = active/warning
#define LED_BLINK_SLOW_MS      1000 // Slow blink = normal
#define LED_OFF_MS             2000 // Off duration for pulses

/*************************************************
 * RELAY LOGIC (ACTIVE LOW)
 *************************************************/
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

#endif