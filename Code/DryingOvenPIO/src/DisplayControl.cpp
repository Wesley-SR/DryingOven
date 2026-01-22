#include "DisplayControl.h"

// Global LCD instance
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Custom character for °C
byte degreeSymbol[8] = {
  B00001100,
  B00010010,
  B00010010,
  B00001100,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

void initDisplay() {
  lcd.init();
  lcd.setBacklight(HIGH);
  lcd.createChar(0, degreeSymbol);
  lcd.clear();

  // Static label layout
  lcd.setCursor(0, 0);
  lcd.print("Temp. : ");
  lcd.setCursor(13, 0);
  lcd.write(byte(0));
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Umid. : ");
  lcd.setCursor(14, 1);
  lcd.print("%");
}

void showWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Drying Oven");
  lcd.setCursor(0, 1);
  lcd.print("LE MANS ST CAR");
  delay(2000);
  initDisplay();  // Return to status layout
}

void updateTemperature(float temperature) {
  lcd.setCursor(8, 0);
  lcd.print("    "); // Clear field
  lcd.setCursor(8, 0);
  lcd.print(temperature, 1); // One decimal
}

void updateHumidity(float humidity) {
  lcd.setCursor(8, 1);
  lcd.print("    "); // Clear field
  lcd.setCursor(8, 1);
  lcd.print(humidity, 1); // One decimal
}
