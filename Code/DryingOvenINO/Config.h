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
 * DHT SENSOR CONFIGURATION
 *************************************************/
#define DHT_TYPE  DHT22

/* Enable sensors */
#define EN_DHT_SENSOR_1 1
#define EN_DHT_SENSOR_2 1
#define EN_DHT_SENSOR_3 0
#define EN_DHT_SENSOR_4 0

#define MAX_DHT_SENSORS 2 // Number of sensors

/* Sensor pins */
#define DHT_PIN_1 9
#define DHT_PIN_2 8
#define DHT_PIN_3 10
// #define DHT_PIN_4 11

/*************************************************
 * RELAY / ACTUATOR PINS
 *************************************************/
#define RESISTENCE_PIN_1  6
#define RESISTENCE_PIN_2  7

#define FAN_PIN_1         4
// #define FAN_PIN_2      3 // Nao utilizado. Estamos conectando as duas FAN com o PIN_1
#define FAN_HUMIDITY_PIN  5

/*************************************************
 * CONTROL PARAMETERS WITH HYSTERESIS
 *************************************************/

/* Temperature control */
#define TEMP_MIN_C        26.0f
#define TEMP_MAX_C        30.0f

/* Humidity control */
#define HUMIDITY_MIN_PCT  80.0f
#define HUMIDITY_MAX_PCT  95.0f

/*************************************************
 * RELAY LOGIC (ACTIVE LOW)
 *************************************************/
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

#endif