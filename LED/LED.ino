#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Replace 0x27 with your address

void setup()
{
  lcd.init();      // Initialize LCD
  lcd.backlight(); // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Hello, ESP8266!");
  lcd.setCursor(0, 1);
  lcd.print("I2C LCD Working!");
}

void loop()
{
  // Nothing here for now
}
