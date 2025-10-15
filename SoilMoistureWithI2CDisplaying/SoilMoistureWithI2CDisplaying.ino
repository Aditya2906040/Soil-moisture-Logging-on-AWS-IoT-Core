#include <LiquidCrystal_I2C.h>
#include<Wire.h>

#define SOIL_PIN A0     // Analog pin

int soilValue = 0;
int moisturePercent = 0;

// Adjust these after calibration
int dryValue = 1024;   // sensor value in dry soil
int wetValue = 400;   // sensor value in wet soil

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("Soil Moisture Sensor Test");
  lcd.init();                      // Initialize LCD
  lcd.backlight();                 // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Hello, ESP8266!");
  lcd.setCursor(0, 1);
  lcd.print("I2C LCD Working!");

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Raw: ");

  delay(100);

  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  delay(2000);

}

void loop() {
  soilValue = analogRead(SOIL_PIN);

  // Map raw value to percentage
  moisturePercent = map(soilValue, dryValue, wetValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw: ");
  Serial.print(soilValue);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  lcd.setCursor(0, 0);
  lcd.print("Raw: ");
  lcd.print(soilValue);

  delay(100);

  lcd.setCursor(0, 1);
  if(moisturePercent < 10)
    lcd.print("0");

  lcd.print("Moisture: ");
  lcd.print(moisturePercent);

  delay(2000);
}
