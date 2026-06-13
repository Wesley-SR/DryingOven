#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class DisplayControl
{
public:
    DisplayControl(uint8_t address, uint8_t cols, uint8_t rows);

    void init();
    void showBootScreen();
    void showLabels();
    
    // Modified: Non-blocking, with error tracking
    bool updateTemperature(float temperature);
    bool updateHumidity(float humidity);
    bool validSensors(uint8_t validCount);
    
    void failMode();
    
    // Error handling
    bool isHealthy();              // Returns true if display working
    void handleDisplayError();     // Called when I2C fails
    uint16_t getErrorCount();      // Returns number of I2C errors
    
    // Diagnostic
    void printDiagnostics();       // Print display status to serial

private:
    LiquidCrystal_I2C lcd;
    void createDegreeSymbol();
    
    // Error tracking
    bool displayConnected;
    uint16_t i2cErrorCount;
    unsigned long lastSuccessfulWrite;
    unsigned long lastErrorTime;
    
    // Safe I2C operations
    bool safeWrite(const char* text, uint8_t col, uint8_t row);
    bool safeClear();
};

#endif