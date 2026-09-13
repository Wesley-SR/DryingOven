// Source: https://arduinoecia.com.br/modulo-i2c-display-16x2-arduino/

#include "config.h"
#include "DisplayControl.h"
#include "SensorTH04S.h"
#include <SoftwareSerial.h>

DisplayControl display(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

SoftwareSerial RS485Serial(RS485_RX_PIN, RS485_TX_PIN);
SensorTH04S sensor1(RS485Serial, TH04S_SENSOR_1_ADDRESS);

/*************************************************
 * STATE MACHINES
 *************************************************/
enum TemperatureState
{
    TEMP_STATE_HEATING_ON = 0,
    TEMP_STATE_HEATING_OFF
};

enum HumidityState
{
    HUM_STATE_DEHUMIDIFY_ON = 0,
    HUM_STATE_DEHUMIDIFY_OFF
};

enum OperationMode
{
    NORMAL = 0,
    NOISE_ROBUSTENESS_TEST
};

static TemperatureState g_temperatureState = TEMP_STATE_HEATING_ON;
static HumidityState g_humidityState = HUM_STATE_DEHUMIDIFY_OFF;
static OperationMode g_operationMode = NORMAL;

// Watchdog tracking for heating duration
static unsigned long heatingStartTime = 0;
static bool heatingWatchdogActive = false;

static uint8_t testMode = 0;

/*************************************************
 * FUNCTION PROTOTYPES
 *************************************************/
static void updateTemperatureStateMachine(float temperature);
static void printTemperatureZone(float temperature);
static void updateHumidityStateMachine(float humidity);
static void applyOutputs(void);
static void applyOutputsNoiseTest(void);
static void setSafeOutputs(void);
static void printStates(float temperature, float humidity);
static bool isTemperatureValid(float temperature);
static bool isHumidityValid(float humidity);
static void updateHeatingWatchdog(void);
static bool readTH04SSensor(float& temperature, float& humidity);
static void printTH04SSensorStatus(float temperature, float humidity);
static const char* getTH04SErrorText(SensorTH04SError error);

void(* resetFunc) (void) = 0;

/*************************************************
 * SETUP
 *************************************************/
void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println(F("Boot"));

    display.init();
    display.showBootScreen();
    Serial.println(F("LE MANS msg"));
    delay(3000);

    Serial.println(F("Init Outputs"));
    pinMode(RESISTENCE_PIN_1, OUTPUT);
    pinMode(RESISTENCE_PIN_2, OUTPUT);
    pinMode(FAN_PIN_1, OUTPUT);
    pinMode(FAN_HUMIDITY_PIN, OUTPUT);

    digitalWrite(FAN_PIN_1, RELAY_OFF);
    digitalWrite(RESISTENCE_PIN_1, RELAY_OFF);
    digitalWrite(RESISTENCE_PIN_2, RELAY_OFF);
    digitalWrite(FAN_HUMIDITY_PIN, RELAY_OFF);

    delay(5000);

    display.showLabels();
    digitalWrite(FAN_PIN_1, RELAY_ON);

    Serial.println(F("Init RS485 sensor"));
    sensor1.begin(RS485_BAUDRATE);

    Serial.println(F("Setup ended"));
}

/*************************************************
 * MAIN LOOP
 *************************************************/
void loop()
{
    delay(MEASUREMENT_INTERVAL_MS);

    float temperature = 0.0f;
    float humidity = 0.0f;

    if (!readTH04SSensor(temperature, humidity))
    {
        Serial.println(F("ERROR: No valid TH04S reading available"));
        Serial.println(F("System switched to safe state"));
        display.failMode();
        heatingWatchdogActive = false;
        Serial.println(F("---------------------------"));
        setSafeOutputs();
        return;
    }

    if (!isTemperatureValid(temperature) || !isHumidityValid(humidity))
    {
        Serial.println(F("ERROR: Invalid sensor reading - using safe state"));
        display.failMode();
        heatingWatchdogActive = false;
        Serial.println(F("---------------------------"));
        setSafeOutputs();
        return;
    }

    display.updateTemperature(temperature);
    display.updateHumidity(humidity);
    display.validSensors(1);

    if (!display.isHealthy())
    {
        Serial.println(F("WARNING: Display communication error - control continues"));
    }

    if (g_operationMode == NOISE_ROBUSTENESS_TEST)
    {
        applyOutputsNoiseTest();
        heatingWatchdogActive = false;
    }
    else
    {
        updateTemperatureStateMachine(temperature);
        updateHumidityStateMachine(humidity);
        updateHeatingWatchdog();
        applyOutputs();
    }

    printStates(temperature, humidity);
}

/*************************************************
 * SENSOR FUNCTIONS
 *************************************************/
static bool readTH04SSensor(float& temperature, float& humidity)
{
    SensorTH04SError result = sensor1.read();

    if (result != SENSOR_TH04S_OK || !sensor1.isValid())
    {
        Serial.print(F("TH04S communication error: "));
        Serial.println(getTH04SErrorText(result));
        return false;
    }

    temperature = sensor1.getTemperature();
    humidity = sensor1.getHumidity();
    printTH04SSensorStatus(temperature, humidity);
    return true;
}

static void printTH04SSensorStatus(float temperature, float humidity)
{
    Serial.print(F("Sensor ID: "));
    Serial.println(sensor1.getAddress());
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F(" C"));
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F(" %RH"));
}

static const char* getTH04SErrorText(SensorTH04SError error)
{
    switch (error)
    {
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

/*************************************************
 * CONTROL FUNCTIONS
 *************************************************/
static void updateTemperatureStateMachine(float temperature)
{
    switch (g_temperatureState)
    {
        case TEMP_STATE_HEATING_ON:
        {
            if (temperature >= TEMP_TURN_OFF_C)
            {
                g_temperatureState = TEMP_STATE_HEATING_OFF;
                Serial.println(F("TEMP STATE: HEATING_ON -> HEATING_OFF"));
            }
            break;
        }

        case TEMP_STATE_HEATING_OFF:
        {
            if (temperature <= TEMP_TURN_ON_C)
            {
                g_temperatureState = TEMP_STATE_HEATING_ON;
                Serial.println(F("TEMP STATE: HEATING_OFF -> HEATING_ON"));
            }
            break;
        }

        default:
        {
            g_temperatureState = TEMP_STATE_HEATING_OFF;
            break;
        }
    }
}

static void printTemperatureZone(float temperature)
{
    if (temperature <= TEMP_TURN_ON_C)
    {
        Serial.print(F(" [ZONE: Below turn-on, "));
    }
    else if (temperature >= TEMP_TURN_OFF_C)
    {
        Serial.print(F(" [ZONE: Above turn-off, "));
    }
    else
    {
        Serial.print(F(" [ZONE: Deadband, "));
    }

    Serial.print(F("state persists"));
    Serial.println(F("]"));
}

static void updateHumidityStateMachine(float humidity)
{
    switch (g_humidityState)
    {
        case HUM_STATE_DEHUMIDIFY_ON:
        {
            if (humidity < HUMIDITY_TURN_OFF_PCT)
            {
                g_humidityState = HUM_STATE_DEHUMIDIFY_OFF;
                Serial.println(F("HUMID STATE: DEHUMIDIFY_ON -> DEHUMIDIFY_OFF"));
            }
            break;
        }

        case HUM_STATE_DEHUMIDIFY_OFF:
        {
            if (humidity > HUMIDITY_TURN_ON_PCT)
            {
                g_humidityState = HUM_STATE_DEHUMIDIFY_ON;
                Serial.println(F("HUMID STATE: DEHUMIDIFY_OFF -> DEHUMIDIFY_ON"));
            }
            break;
        }

        default:
        {
            g_humidityState = HUM_STATE_DEHUMIDIFY_OFF;
            break;
        }
    }
}

static void applyOutputs(void)
{
    if (g_temperatureState == TEMP_STATE_HEATING_ON)
    {
        digitalWrite(RESISTENCE_PIN_1, RELAY_ON);
        digitalWrite(RESISTENCE_PIN_2, RELAY_ON);
    }
    else
    {
        digitalWrite(RESISTENCE_PIN_1, RELAY_OFF);
        digitalWrite(RESISTENCE_PIN_2, RELAY_OFF);
    }

    if (g_humidityState == HUM_STATE_DEHUMIDIFY_ON)
    {
        digitalWrite(FAN_HUMIDITY_PIN, RELAY_ON);
    }
    else
    {
        digitalWrite(FAN_HUMIDITY_PIN, RELAY_OFF);
    }
}

static void applyOutputsNoiseTest(void)
{
    if (testMode == 0)
    {
        digitalWrite(RESISTENCE_PIN_1, RELAY_ON);
        digitalWrite(RESISTENCE_PIN_2, RELAY_ON);
        digitalWrite(FAN_HUMIDITY_PIN, RELAY_ON);
        digitalWrite(FAN_PIN_1, RELAY_ON);
        testMode = 1;
    }
    else
    {
        digitalWrite(RESISTENCE_PIN_1, RELAY_OFF);
        digitalWrite(RESISTENCE_PIN_2, RELAY_OFF);
        digitalWrite(FAN_HUMIDITY_PIN, RELAY_OFF);
        digitalWrite(FAN_PIN_1, RELAY_OFF);
        testMode = 0;
    }
}

static void setSafeOutputs(void)
{
    digitalWrite(RESISTENCE_PIN_1, RELAY_OFF);
    digitalWrite(RESISTENCE_PIN_2, RELAY_OFF);
    digitalWrite(FAN_HUMIDITY_PIN, RELAY_OFF);
    digitalWrite(FAN_PIN_1, RELAY_OFF);

    delay(10000);
    resetFunc();
}

static void printStates(float temperature, float humidity)
{
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F(" C | Humidity: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    Serial.print(F("Temperature state: "));
    if (g_temperatureState == TEMP_STATE_HEATING_ON)
    {
        Serial.println(F("HEATING_ON"));
    }
    else
    {
        Serial.println(F("HEATING_OFF"));
    }

    Serial.print(F("Humidity state: "));
    printTemperatureZone(temperature);

    if (g_humidityState == HUM_STATE_DEHUMIDIFY_ON)
    {
        Serial.println(F("DEHUMIDIFY_ON"));
    }
    else
    {
        Serial.println(F("DEHUMIDIFY_OFF"));
    }

    // Print display diagnostics approximately every five minutes
    static uint16_t loopCounter = 0;
    loopCounter++;

    if (loopCounter >= 60)
    {
        display.printDiagnostics();
        loopCounter = 0;
    }

    Serial.println(F("---------------------------"));
}

static bool isTemperatureValid(float temperature)
{
    if (temperature < TEMP_SENSOR_MIN_VALID || temperature > TEMP_SENSOR_MAX_VALID)
    {
        Serial.print(F("ALERT: Temperature out of valid range: "));
        Serial.println(temperature);
        return false;
    }

    return true;
}

static bool isHumidityValid(float humidity)
{
    if (humidity < 0.0f || humidity > 100.0f)
    {
        Serial.print(F("ALERT: Humidity out of valid range: "));
        Serial.print(humidity);
        Serial.println(F(" % (valid: 0-100)"));
        return false;
    }

    return true;
}

static void updateHeatingWatchdog(void)
{
    if (g_temperatureState == TEMP_STATE_HEATING_ON)
    {
        if (!heatingWatchdogActive)
        {
            heatingStartTime = millis();
            heatingWatchdogActive = true;
            Serial.println(F("INFO: Heating watchdog started"));
        }

        unsigned long heatingDuration = millis() - heatingStartTime;

        if (heatingDuration > HEATING_WATCHDOG_MS)
        {
            Serial.println(F("WARNING: HEATING WATCHDOG TRIGGERED!"));
            Serial.print(F("Heating active for "));
            Serial.print(heatingDuration / 1000);
            Serial.println(F(" seconds - forcing OFF for safety"));

            g_temperatureState = TEMP_STATE_HEATING_OFF;
            heatingWatchdogActive = false;
        }
    }
    else if (heatingWatchdogActive)
    {
        unsigned long heatingDuration = millis() - heatingStartTime;
        Serial.print(F("INFO: Heating OFF after "));
        Serial.print(heatingDuration / 1000);
        Serial.println(F(" seconds"));
        heatingWatchdogActive = false;
    }
}
