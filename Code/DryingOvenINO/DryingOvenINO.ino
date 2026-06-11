// Source: https://arduinoecia.com.br/sensor-de-temperatura-e-umidade-dht22/?srsltid=AfmBOorTfJ_5RpiVozhTu0l7Cj7TuW3k1dwqq2iNTbNCwK_GeuiEmHlJ
// Source: https://arduinoecia.com.br/modulo-i2c-display-16x2-arduino/

#include "config.h"
#include "DisplayControl.h"
#include "SensorDHT.h"

DisplayControl display(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
SensorDHT dhtSensor;

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

bool temperatureStateChanged = false;
bool humidityStateChanged = false;

static OperationMode g_operationMode = NORMAL;

uint8_t testMode = 0;

/*************************************************
 * FUNCTION PROTOTYPES
 *************************************************/
static void updateTemperatureStateMachine(float temperature);
static void updateHumidityStateMachine(float humidity);
static void applyOutputs(void);
static void applyOutputsNoiseTest(void);
static void setSafeOutputs(void);
static void printSensorStatus(const DHTReading readings[MAX_DHT_SENSORS], uint8_t validCount);
static void printStates(float temperature, float humidity);

/*************************************************
 * SETUP
 *************************************************/
void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("Boot");
    display.init();
    display.showBootScreen();
    Serial.println("LE MANS msg");
    delay(3000);

    Serial.println("Init Outputs");
    pinMode(RESISTENCE_PIN_1, OUTPUT);
    pinMode(RESISTENCE_PIN_2, OUTPUT);
    pinMode(FAN_PIN_1, OUTPUT);
    pinMode(FAN_HUMIDITY_PIN, OUTPUT);

    digitalWrite(FAN_PIN_1, RELAY_OFF);
    digitalWrite(RESISTENCE_PIN_1, RELAY_OFF);
    digitalWrite(RESISTENCE_PIN_2, RELAY_OFF);
    digitalWrite(FAN_HUMIDITY_PIN, RELAY_OFF);
    delay(5000);

    Serial.println("Init Sensors");
    dhtSensor.begin();

    display.showLabels();
    Serial.println("Setup ended");

    digitalWrite(FAN_PIN_1, RELAY_ON);
}

/*************************************************
 * MAIN LOOP
 *************************************************/
void loop()
{
    delay(MEASUREMENT_INTERVAL_MS);

    DHTReading readings[MAX_DHT_SENSORS];
    float temperature = 0.0f;
    float humidity = 0.0f;
    uint8_t validCount = 0;

    bool hasValidSensor = dhtSensor.readAll(readings, temperature, humidity, validCount);

    printSensorStatus(readings, validCount);

    if (!hasValidSensor)
    {
        Serial.println("Falha: nenhum sensor DHT valido disponivel!");
        Serial.println("Sistema colocado em estado seguro.");
        setSafeOutputs();
        display.failMode();
        Serial.println("---------------------------");
        return;
    }

    display.updateTemperature(temperature);
    display.updateHumidity(humidity);
    display.validSensors(validCount);

    if (g_operationMode == NOISE_ROBUSTENESS_TEST)
    {
        applyOutputsNoiseTest();
    }
    else // (g_operationMode == NORMAL)
    {
        // Control
        updateTemperatureStateMachine(temperature);
        updateHumidityStateMachine(humidity);
        // Update control outputs
        applyOutputs(); // Here is where the relays switch
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
            if (temperature >= TEMP_MAX_C)
            {
                g_temperatureState = TEMP_STATE_HEATING_OFF;
            }
            break;
        }

        case TEMP_STATE_HEATING_OFF:
        {
            if (temperature <= TEMP_MIN_C)
            {
                g_temperatureState = TEMP_STATE_HEATING_ON;
            }
            break;
        }

        default:
        {
            g_temperatureState = TEMP_STATE_HEATING_OFF;
            break;
        }
    }

    // Track state changes
    if (g_temperatureState != g_temperatureLastState)
    {
        temperatureStateChanged = true;
    }
    else
    {
        temperatureStateChanged = false;
    }
}

static void updateHumidityStateMachine(float humidity)
{
    switch (g_humidityState)
    {
        case HUM_STATE_DEHUMIDIFY_ON:
        {
            if (humidity <= HUMIDITY_MIN_PCT)
            {
                g_humidityState = HUM_STATE_DEHUMIDIFY_OFF;
            }
            break;
        }

        case HUM_STATE_DEHUMIDIFY_OFF:
        {
            if (humidity >= HUMIDITY_MAX_PCT)
            {
                g_humidityState = HUM_STATE_DEHUMIDIFY_ON;
            }
            break;
        }

        default:
        {
            g_humidityState = HUM_STATE_DEHUMIDIFY_OFF;
            break;
        }
    }

    // Track state changes
    if (g_humidityState != g_humidityLastState)
    {
        humidityStateChanged = true;
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
}

static void printSensorStatus(const DHTReading readings[MAX_DHT_SENSORS], uint8_t validCount)
{
    uint8_t enabledCount = 0;
    uint8_t failedCount = 0;

    for (int i = 0; i < MAX_DHT_SENSORS; i++)
    {
        if (readings[i].enabled)
        {
            enabledCount++;

            Serial.print("Sensor ");
            Serial.print(i + 1);

            if (readings[i].valid)
            {
                Serial.print(": OK | T=");
                Serial.print(readings[i].temperature);
                Serial.print(" C | H=");
                Serial.print(readings[i].humidity);
                Serial.println(" %");
            }
            else
            {
                failedCount++;
                Serial.println(": FALHA");
            }
        }
    }

    Serial.print("Validos: ");
    Serial.print(validCount);
    Serial.print(" | Falhando: ");
    Serial.println(failedCount);
}

static void printStates(float temperature, float humidity)
{
    Serial.print("Temperatura media: ");
    Serial.print(temperature);
    Serial.print(" C | Umidade media: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Estado Temperatura: ");
    if (g_temperatureState == TEMP_STATE_HEATING_ON)
    {
        Serial.println("HEATING_ON");
    }
    else
    {
        Serial.println("HEATING_OFF");
    }

    Serial.print("Estado Umidade: ");
    if (g_humidityState == HUM_STATE_DEHUMIDIFY_ON)
    {
        Serial.println("DEHUMIDIFY_ON");
    }
    else
    {
        Serial.println("DEHUMIDIFY_OFF");
    }

    Serial.println("---------------------------");
}