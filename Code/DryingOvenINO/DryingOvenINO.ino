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

// LED blink timing
static unsigned long lastLedUpdate = 0;
static bool ledBlinkState = false;
static uint8_t sensorErrorCount = 0;

// Watchdog tracking for heating duration
static unsigned long heatingStartTime = 0;
static bool heatingWatchdogActive = false;

uint8_t testMode = 0;

/*************************************************
 * FUNCTION PROTOTYPES
 *************************************************/
static void updateTemperatureStateMachine(float temperature);
static void printTemperatureZone(float temperature);
static void updateHumidityStateMachine(float humidity);
static void applyOutputs(void);
static void applyOutputsNoiseTest(void);
static void setSafeOutputs(void);
static void printSensorStatus(const DHTReading readings[MAX_DHT_SENSORS], uint8_t validCount);
static void printStates(float temperature, float humidity);
static bool isTemperatureValid(float temperature);
static bool isHumidityValid(float humidity);
static void updateHeatingWatchdog(void);
static bool updateLedBlink(bool fast);
static void updateStatusLeds(float temperature, float humidity, uint8_t validCount);
static void trackSensorErrors(uint8_t validCount, uint8_t enabledCount);
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
    
    // NEW (PATCH-005): Init status LEDs
#if ENABLE_STATUS_LEDS
    Serial.println(F("Init Status LEDs"));
    if (STATUS_LED_GREEN_PIN >= 0)
        pinMode(STATUS_LED_GREEN_PIN, OUTPUT);
    if (STATUS_LED_RED_PIN >= 0)
        pinMode(STATUS_LED_RED_PIN, OUTPUT);
    if (HEATING_LED_PIN >= 0)
        pinMode(HEATING_LED_PIN, OUTPUT);
    if (SENSOR_LED_PIN >= 0)
        pinMode(SENSOR_LED_PIN, OUTPUT);
    
    // Clear all LEDs
    if (STATUS_LED_GREEN_PIN >= 0)
        digitalWrite(STATUS_LED_GREEN_PIN, LOW);
    if (STATUS_LED_RED_PIN >= 0)
        digitalWrite(STATUS_LED_RED_PIN, LOW);
    if (HEATING_LED_PIN >= 0)
        digitalWrite(HEATING_LED_PIN, LOW);
    if (SENSOR_LED_PIN >= 0)
        digitalWrite(SENSOR_LED_PIN, LOW);
#endif
    
    delay(5000);

    Serial.println(F("Init Sensors"));
    dhtSensor.begin();

    display.showLabels();
    Serial.println(F("Setup ended"));

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
        Serial.println(F("Falha: nenhum sensor DHT valido disponivel!"));
        
        Serial.println(F("Sistema colocado em estado seguro."));
        display.failMode();
        heatingWatchdogActive = false;  // NEW (PATCH-002)
        Serial.println(F("---------------------------"));
        setSafeOutputs();
        return;
    }

    // NEW (PATCH-002): Validate temperature and humidity
    if (!isTemperatureValid(temperature) || !isHumidityValid(humidity))
    {
        Serial.println(F("ERROR: Invalid sensor reading - using safe state"));
        setSafeOutputs();
        heatingWatchdogActive = false;
        Serial.println(F("---------------------------"));
        return;
    }

    display.updateTemperature(temperature);
    display.updateHumidity(humidity);
    display.validSensors(validCount);

    // Check display health
    if (!display.isHealthy())
    {
        Serial.println(F("WARNING: Display communication error - control continues"));
    }

    if (g_operationMode == NOISE_ROBUSTENESS_TEST)
    {
        applyOutputsNoiseTest();
        heatingWatchdogActive = false;
    }
    else // (g_operationMode == NORMAL)
    {
        // Control
        updateTemperatureStateMachine(temperature);
        updateHumidityStateMachine(humidity);
        
        // NEW (PATCH-002): Update heating watchdog
        updateHeatingWatchdog();
        
        // Update control outputs
        applyOutputs();
    }
    
    printStates(temperature, humidity);
    
    // NEW (PATCH-005): Update status LEDs
    updateStatusLeds(temperature, humidity, validCount);
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
            // Uses hysteresis upper threshold
            if (temperature >= TEMP_TURN_OFF_C)
            {
                g_temperatureState = TEMP_STATE_HEATING_OFF;
                Serial.println(F("TEMP STATE: HEATING_ON -> HEATING_OFF"));
            }
            break;
        }

        case TEMP_STATE_HEATING_OFF:
        {
            // Uses hysteresis lower threshold
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

    // Track state changes
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

/**
 * Print temperature zone information (for debugging hysteresis)
 */
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

    // Track state changes
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

static void printSensorStatus(const DHTReading readings[MAX_DHT_SENSORS], uint8_t validCount)
{
    uint8_t enabledCount = 0;
    uint8_t failedCount = 0;

    for (int i = 0; i < MAX_DHT_SENSORS; i++)
    {
        if (readings[i].enabled)
        {
            enabledCount++;

            Serial.print(F("Sensor "));
            Serial.print(i + 1);

            if (readings[i].valid)
            {
                Serial.print(F(": OK | T="));
                Serial.print(readings[i].temperature);
                Serial.print(F(" C | H="));
                Serial.print(readings[i].humidity);
                Serial.println(F(" %"));
            }
            else
            {
                failedCount++;
                Serial.println(F(": FALHA"));
            }
        }
    }

    Serial.print(F("Validos: "));
    Serial.print(validCount);
    Serial.print(F(" | Falhando: "));
    Serial.println(failedCount);
}

static void printStates(float temperature, float humidity)
{
    Serial.print(F("Temperatura media: "));
    Serial.print(temperature);
    Serial.print(F(" C | Umidade media: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    Serial.print(F("Estado Temperatura: "));
    if (g_temperatureState == TEMP_STATE_HEATING_ON)
    {
        Serial.println(F("HEATING_ON"));
    }
    else
    {
        Serial.println(F("HEATING_OFF"));
    }

    Serial.print(F("Estado Umidade: "));
    printTemperatureZone(temperature);
    if (g_humidityState == HUM_STATE_DEHUMIDIFY_ON)
    {
        Serial.println(F("DEHUMIDIFY_ON"));
    }
    else
    {
        Serial.println(F("DEHUMIDIFY_OFF"));
    }

    // Periodic display diagnostics (every 60 iterations = ~5 minutes)
    static uint16_t loopCounter = 0;
    loopCounter++;
    if (loopCounter >= 60)
    {
        display.printDiagnostics();
        loopCounter = 0;
    }

    Serial.println(F("---------------------------"));
}

// Temperature/Humidity Validation
static bool isTemperatureValid(float temperature)
{
    if (temperature < TEMP_SENSOR_MIN_VALID || 
        temperature > TEMP_SENSOR_MAX_VALID)
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

// Heating Watchdog
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
    else
    {
        if (heatingWatchdogActive)
        {
            unsigned long heatingDuration = millis() - heatingStartTime;
            Serial.print(F("INFO: Heating OFF after "));
            Serial.print(heatingDuration / 1000);
            Serial.println(F(" seconds"));
            heatingWatchdogActive = false;
        }
    }
}

/**
 * Update LED blink state (for pulsing/blinking effects)
 * Returns: true if LED should be ON, false if OFF
 */
static bool updateLedBlink(bool fast)
{
    unsigned long interval = fast ? LED_BLINK_FAST_MS : LED_BLINK_SLOW_MS;
    unsigned long totalCycle = interval * 2;
    
    unsigned long elapsed = millis() % totalCycle;
    return (elapsed < interval);
}


/**
 * Update all status LEDs based on system state
 * Called once per loop iteration
 */
static void updateStatusLeds(float temperature, float humidity, uint8_t validCount)
{
#if !ENABLE_STATUS_LEDS
    return;
#endif

    unsigned long currentTime = millis();
    if (currentTime - lastLedUpdate < 100)  // Update LEDs every 100ms
    {
        return;
    }
    lastLedUpdate = currentTime;

    // LED 1: Green = System OK & Operating Normally
    if (STATUS_LED_GREEN_PIN >= 0)
    {
        bool systemHealthy = (validCount > 0) && 
                            (g_temperatureState != 99) &&  // Not in error state
                            (isTemperatureValid(temperature));
        
        if (systemHealthy)
        {
            digitalWrite(STATUS_LED_GREEN_PIN, HIGH);  // ON = good
        }
        else
        {
            digitalWrite(STATUS_LED_GREEN_PIN, LOW);
        }
    }

    // LED 2: Red = Error Detected
    if (STATUS_LED_RED_PIN >= 0)
    {
        bool hasError = (validCount == 0) ||  // No valid sensors
                       (!isTemperatureValid(temperature)) ||  // Invalid reading
                       (sensorErrorCount > 0);  // Recent sensor errors
        
        if (hasError)
        {
            // Blink red rapidly if error
            bool blinkState = updateLedBlink(true);
            digitalWrite(STATUS_LED_RED_PIN, blinkState ? HIGH : LOW);
            
            // Clear error after error is resolved
            if (!hasError)
            {
                sensorErrorCount = 0;
            }
        }
        else
        {
            digitalWrite(STATUS_LED_RED_PIN, LOW);
        }
    }

    // LED 3: Heating Active
    if (HEATING_LED_PIN >= 0)
    {
        bool heatingActive = (g_temperatureState == TEMP_STATE_HEATING_ON);
        
        if (heatingActive)
        {
            // Solid ON when heating
            digitalWrite(HEATING_LED_PIN, HIGH);
        }
        else
        {
            // Slow blink when heating OFF (monitoring)
            bool blinkState = updateLedBlink(false);
            digitalWrite(HEATING_LED_PIN, blinkState ? HIGH : LOW);
        }
    }

    // LED 4: Sensor Health
    if (SENSOR_LED_PIN >= 0)
    {
        bool sensorsGood = (validCount >= 1);  // At least one valid sensor
        
        if (sensorsGood)
        {
            digitalWrite(SENSOR_LED_PIN, HIGH);  // Solid ON if sensors OK
        }
        else
        {
            // Fast blink if sensor problems
            bool blinkState = updateLedBlink(true);
            digitalWrite(SENSOR_LED_PIN, blinkState ? HIGH : LOW);
        }
    }
}

/**
 * Track sensor read errors for LED indication
 */
static void trackSensorErrors(uint8_t validCount, uint8_t enabledCount)
{
    if (validCount < enabledCount)
    {
        sensorErrorCount = enabledCount - validCount;
    }
    else
    {
        sensorErrorCount = 0;
    }
}