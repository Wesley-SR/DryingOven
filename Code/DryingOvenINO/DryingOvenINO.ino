// Source: https://arduinoecia.com.br/modulo-i2c-display-16x2-arduino/

#include "config.h"
#include "DisplayControl.h"
#include "SensorTH04SManager.h"
#include <SoftwareSerial.h>

DisplayControl display(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

/*************************************************
 * RS485 SENSORS
 *************************************************/
SoftwareSerial RS485Serial(RS485_RX_PIN, RS485_TX_PIN);
SensorTH04S sensor1(RS485Serial, RS485_DE_RE_PIN, TH04S_SENSOR_1_ADDRESS);
unsigned long lastSensorRead = 0;

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
static TemperatureState g_temperatureLastState = TEMP_STATE_HEATING_ON;
static HumidityState g_humidityLastState = HUM_STATE_DEHUMIDIFY_OFF;
static OperationMode g_operationMode = NORMAL;

bool temperatureStateChanged = false;
bool humidityStateChanged = false;

// Watchdog tracking for heating duration
static unsigned long heatingStartTime = 0;
static bool heatingWatchdogActive = false;

uint8_t testMode = 0;


/*************************************************
 * SOME CONFIGURATIONS
 *************************************************/
static OperationMode g_operationMode = NORMAL;


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
static void updateHeatingWatchdog(void);
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
    delay(3000);

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

    sensorManager.begin(RS485_BAUDRATE);

    Serial.println(F("Setup ended"));
}

/*************************************************
 * MAIN LOOP
 *************************************************/
void loop()
{
    delay(MEASUREMENT_INTERVAL_MS);

    if (!sensorManager.readAll())
    {
        Serial.println(F("ERROR: No valid TH04S sensor available"));
        display.failMode();
        heatingWatchdogActive = false;
        setSafeOutputs();
        return;
    }

    float temperature = sensorManager.getAverageTemperature();
    float humidity = sensorManager.getAverageHumidity();
    uint8_t validSensors = sensorManager.getValidSensorCount();

    display.updateTemperature(temperature);
    display.updateHumidity(humidity);
    display.validSensors(validSensors);

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
 * FUNCTIONS
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

    if (g_temperatureState != g_temperatureLastState)
    {
        temperatureStateChanged = true;
        g_temperatureLastState = g_temperatureState;
    }
    else
    {
        temperatureStateChanged = false;
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

    if (g_humidityState != g_humidityLastState)
    {
        humidityStateChanged = true;
        g_humidityLastState = g_humidityState;
    }
    else
    {
        humidityStateChanged = false;
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
    Serial.print(F("Average temperature: "));
    Serial.print(temperature);
    Serial.print(F(" C | Average humidity: "));
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

    static uint16_t loopCounter = 0;
    loopCounter++;
    if (loopCounter >= 60)
    {
        display.printDiagnostics();
        loopCounter = 0;
    }

    Serial.println(F("---------------------------"));
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
