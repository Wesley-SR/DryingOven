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
    void updateTemperature(float temperature);
    void updateHumidity(float humidity);
    void failMode();
    void validSensors(uint8_t validCount);

private:
    LiquidCrystal_I2C lcd;
    void createDegreeSymbol();
};

#endif
