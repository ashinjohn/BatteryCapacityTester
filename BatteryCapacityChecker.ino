/*
   Battery Capacity Checker

   Description: Estimates the capacity of batteries by discharging them to batterylow threshold voltage.
   Firmware fine tuned for 18650 Lithium Battery
   4 channels supporting 30V or 3A ( Not both at a time, Heat dissipation issues)
   IDE -> Arduino
   MCU -> ATmega328P Barebone Arduino

   Hardware: Dummy Load Circuit, 16x 2 LCD Module
   Revision- 1.0

*/

/* Probe Color and Battery
   RED -> B1
   YELLOW -> B2
   GREEN -> B2
   ORANGE -> B2
*/

// 16x 2 LCD Configuration HD44780
#include <LiquidCrystal.h>
const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Battery Voltage Measurement
int BAT_V1 = A5, BAT_V2 = A4, BAT_V3 = A3, BAT_V4 = A2;

float GetBATVoltage(int Battery)
{
  return (analogRead(Battery) * (5.0 / 1024.0));
}

void setup() {
  // Print Code Revision to the LCD.
  lcd.begin(16, 2);
  lcd.print("Battery Checker");
  lcd.setCursor(0, 1);
  lcd.print("R 1.0");
  delay(1000);
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(GetBATVoltage(BAT_V4));
}
