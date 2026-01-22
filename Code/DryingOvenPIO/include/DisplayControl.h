#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Initialize display constants
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS    2

void initDisplay();
void updateTemperature(float temperature);
void updateHumidity(float humidity);
void showWelcomeMessage();

#endif
