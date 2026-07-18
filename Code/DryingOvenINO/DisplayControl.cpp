#include "DisplayControl.h"
#include "config.h"
#include <Wire.h>

// Custom degree symbol
static byte degreeSymbol[8] = {
    B00001100,
    B00010010,
    B00010010,
    B00001100,
    B00000000,
    B00000000,
    B00000000,
    B00000000
};

DisplayControl::DisplayControl(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows),
      displayConnected(false),
      i2cErrorCount(0),
      lastSuccessfulWrite(0),
      lastErrorTime(0)
{
}


void DisplayControl::init()
{
    // Ativa o timeout nativo do hardware I2C para evitar travamento total
    #if defined(TWCR) && defined(TWTO)
        Wire.setWireTimeout(25000, true); // 25ms max antes de liberar o barramento
    #endif

    lcd.init();
    
    // Verifica se a inicialização física respondeu no barramento
    #if defined(TWCR) && defined(TWTO)
        if (Wire.getWireTimeoutFlag()) {
            displayConnected = false;
            handleDisplayError();
            return;
        }
    #endif

    lcd.setBacklight(HIGH);
    createDegreeSymbol();
    
    displayConnected = true;
    lastSuccessfulWrite = millis();
    
    Serial.println(F("Display initialized successfully"));
    
#if ENABLE_DISPLAY_LED
    pinMode(DISPLAY_ERROR_LED_PIN, OUTPUT);
    digitalWrite(DISPLAY_ERROR_LED_PIN, LOW);  // LED desligado ou Verde = OK
#endif
}

void DisplayControl::createDegreeSymbol()
{
    lcd.createChar(0, degreeSymbol);
}

void DisplayControl::showBootScreen()
{
    safeClear();
    safeWrite("Drying Oven", 0, 0);
    safeWrite("LE MANS ST CAR", 0, 1);
}

void DisplayControl::showLabels()
{
    safeClear();
    safeWrite("Temp: ", 0, 0);
    
    if (displayConnected)
    {
        lcd.setCursor(11, 0);
        lcd.write(byte(0));
        lcd.print("C");
        
        #if defined(TWCR) && defined(TWTO)
        if (Wire.getWireTimeoutFlag()) {
            handleDisplayError();
            return;
        }
        #endif
    }

    safeWrite("Umid: ", 0, 1);
    
    if (displayConnected)
    {
        lcd.setCursor(12, 1);
        lcd.print("%");
        
        #if defined(TWCR) && defined(TWTO)
        if (Wire.getWireTimeoutFlag()) {
            handleDisplayError();
            return;
        }
        #endif
    }
}

// Escrita segura e não-bloqueante
bool DisplayControl::safeWrite(const char* text, uint8_t col, uint8_t row)
{
    if (!displayConnected)
    {
        return false;
    }

    lcd.setCursor(col, row);
    lcd.print("    ");
    lcd.setCursor(col, row);
    lcd.print(text);
    
    #if defined(TWCR) && defined(TWTO)
    if (Wire.getWireTimeoutFlag())
    {
        handleDisplayError();
        return false;
    }
    #endif
    
    lastSuccessfulWrite = millis();
    return true;
}

// Limpeza de tela segura e não-bloqueante
bool DisplayControl::safeClear()
{
    if (!displayConnected)
    {
        return false;
    }
    
    lcd.clear();
    
    #if defined(TWCR) && defined(TWTO)
    if (Wire.getWireTimeoutFlag())
    {
        handleDisplayError();
        return false;
    }
    #endif
    
    return true;
}

// Atualização de temperatura sem risco de travar a estufa
bool DisplayControl::updateTemperature(float temperature)
{
    if (!displayConnected)
    {
        return false;
    }

    lcd.setCursor(6, 0);
    lcd.print("    ");
    lcd.setCursor(6, 0);
    lcd.print(temperature);
    
    #if defined(TWCR) && defined(TWTO)
    if (Wire.getWireTimeoutFlag())
    {
        handleDisplayError();
        return false;
    }
    #endif
    
    lastSuccessfulWrite = millis();
    return true;
}

// MODIFIED: Non-blocking humidity update
bool DisplayControl::updateHumidity(float humidity)
{
    if (!displayConnected)
    {
        return false;
    }

    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(6, 1);
    lcd.print(humidity);
    
    #if defined(TWCR) && defined(TWTO)
    if (Wire.getWireTimeoutFlag())
    {
        handleDisplayError();
        return false;
    }
    #endif
    
    lastSuccessfulWrite = millis();
    return true;
}

// Non-blocking sensor count update
bool DisplayControl::validSensors(uint8_t validCount)
{
    if (!displayConnected)
    {
        return false;
    }

    lcd.setCursor(14, 0);
    lcd.print(validCount);
    lcd.print("S");
    
    #if defined(TWCR) && defined(TWTO)
    if (Wire.getWireTimeoutFlag())
    {
        handleDisplayError();
        return false;
    }
    #endif
    
    lastSuccessfulWrite = millis();
    return true;
}

void DisplayControl::failMode()
{
    if (displayConnected)
    {
        safeClear();
        safeWrite("Sensor com falha", 0, 0);
    }
}

// NEW: Health check
bool DisplayControl::isHealthy()
{
    // Considera o display saudável se não houver erros recentes (últimos 5 segundos)
    if (i2cErrorCount > 0 && (millis() - lastErrorTime < 5000))
    {
        return false;
    }
    return displayConnected;
}

// NEW: Error handler
void DisplayControl::handleDisplayError()
{
    i2cErrorCount++;
    lastErrorTime = millis();
    displayConnected = false; // Desconecta para evitar novas tentativas pesadas no I2C
    
    #if defined(TWCR) && defined(TWTO)
        Wire.clearWireTimeoutFlag(); // Limpa a flag do hardware para tentar recuperar depois
    #endif

    Serial.print(F("[ERRO I2C] Falha de comunicação com o display. Total: "));
    Serial.println(i2cErrorCount);

#if ENABLE_DISPLAY_LED
    digitalWrite(DISPLAY_ERROR_LED_PIN, HIGH); // Acende o LED de alerta (Vermelho/Alerta)
#endif
}

// NEW: Get error count
uint16_t DisplayControl::getErrorCount()
{
    return i2cErrorCount;
}

// NEW: Diagnostics
void DisplayControl::printDiagnostics()
{
    Serial.println(F("====== DIAGNOSTICO DO DISPLAY ======"));
    Serial.print(F("Conectado: ")); Serial.println(displayConnected ? F("SIM") : F("NAO"));
    Serial.print(F("Saudavel: ")); Serial.println(isHealthy() ? F("SIM") : F("NAO"));
    Serial.print(F("Total de Erros I2C: ")); Serial.println(i2cErrorCount);
    Serial.print(F("Ultima escrita com sucesso (ms): ")); Serial.println(lastSuccessfulWrite);
    Serial.print(F("Ultimo erro registrado (ms): ")); Serial.println(lastErrorTime);
    Serial.println(F("===================================="));
}