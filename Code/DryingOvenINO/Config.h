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
#define ENABLE_DISPLAY_LED    1      // Set to 0 if no LED available

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

/*************************************************
 * SENSOR TIMEOUT CONFIGURATION
 *************************************************/
#define DHT_READ_TIMEOUT_MS   2500
#define DHT_STALL_THRESHOLD   3

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
#define TEMP_TURN_ON_C     24.0f   // Start heating
#define TEMP_TURN_OFF_C    32.0f   // Stop heating
#define TEMP_HYSTERESIS_C  (TEMP_TURN_OFF_C - TEMP_TURN_ON_C)

#define HUMIDITY_TURN_ON_PCT   85.0f   // Start dehumidifying below this
#define HUMIDITY_TURN_OFF_PCT  92.0f   // Stop dehumidifying above this

/*************************************************
 * SAFETY WATCHDOG & VALIDATION
 *************************************************/
#define TEMP_SENSOR_MIN_VALID  -40.0f   // DHT22 valid range
#define TEMP_SENSOR_MAX_VALID  +80.0f   // DHT22 valid range
#define HEATING_WATCHDOG_MS    120000   // 120 seconds max heating

/*************************************************
 * SAFETY WATCHDOG & VALIDATION
 *************************************************/
#define TEMP_SENSOR_MIN_VALID  -40.0f
#define TEMP_SENSOR_MAX_VALID  +80.0f
#define HEATING_WATCHDOG_MS    120000

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