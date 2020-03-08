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

// 16x 2 LCD Configuration HD44780
#include <LiquidCrystal.h>
const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Battery Voltage Measurement
volatile float BAT_VR = 0.0, BAT_VY = 0.0, BAT_VG = 0.0, BAT_VO = 0.0;
int VR = A5, VY = A4, VG = A3, VO = A2; //Pins Mapped to Battery Voltage

float GetBATVoltage(int Battery)
{
  return (analogRead(Battery) * (5.0 / 1024.0));
}

void RefreshBATVoltage()
{
  Serial.println();
  Serial.println("Battery Voltages");
  BAT_VR = GetBATVoltage(VR); delay(100); Serial.print("BAT Voltage R :"); Serial.println(BAT_VR);
  BAT_VY = GetBATVoltage(VY); delay(100); Serial.print("BAT Voltage Y :"); Serial.println(BAT_VY);
  BAT_VG = GetBATVoltage(VG); delay(100); Serial.print("BAT Voltage G :"); Serial.println(BAT_VG);
  BAT_VO = GetBATVoltage(VO); delay(100); Serial.print("BAT Voltage O :"); Serial.println(BAT_VO);
  Serial.println();
}


void setup() {
  // Print Code Revision to the LCD.
  Serial.begin(9600);
  Serial.println("Battery Checker");
  lcd.begin(16, 2);
  lcd.print("Battery Checker");
  Serial.println("R 1.0");
  lcd.setCursor(0, 1);
  lcd.print("R 1.0");
  delay(1000);
}

void loop() {
  RefreshBATVoltage();
  delay(100);
}
