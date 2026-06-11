#include "DisplayControl.h"

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
    : lcd(address, cols, rows)
{
}

void DisplayControl::init()
{
    lcd.init();
    lcd.setBacklight(HIGH);
    createDegreeSymbol();
}

void DisplayControl::createDegreeSymbol()
{
    lcd.createChar(0, degreeSymbol);
}

void DisplayControl::showBootScreen()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Drying Oven");
    lcd.setCursor(0, 1);
    lcd.print("LE MANS ST CAR");
}

void DisplayControl::showLabels()
{
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.setCursor(11, 0);
    lcd.write(byte(0));
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.setCursor(12, 1);
    lcd.print("%");
}



void DisplayControl::updateTemperature(float temperature)
{
    lcd.setCursor(6, 0);
    lcd.print("    ");
    lcd.setCursor(6, 0);
    lcd.print(temperature);
}

void DisplayControl::updateHumidity(float humidity)
{
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(6, 1);
    lcd.print(humidity);
}

void DisplayControl::validSensors(uint8_t validSensorsCount)
{
    lcd.setCursor(14, 0);
    lcd.print(validSensorsCount);
    lcd.print("S");
}

void DisplayControl::failMode()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor failure");
}
