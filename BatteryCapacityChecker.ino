/*
   Battery Capacity Checker

   Description: Estimates the capacity of batteries by discharging them to batterylow threshold voltage.
   Firmware fine tuned for 18650 Lithium Battery
   4 channels supporting 30V or 3A ( Not both at a time, Heat dissipation issues)
   IDE -> Arduino
   MCU -> ATmega328P Barebone Arduino

   Hardware: Dummy Load Circuit, 16x 2 LCD Module
   Revision- 1.0

   Timer Library : https://github.com/JChristensen/Timer

*/

// 16x 2 LCD Configuration HD44780
#include <LiquidCrystal.h>
const int rs = 4, en = 3, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include "Timer.h"
Timer t;

//Battery AH
float AH_R = 0, AH_Y = 0, AH_G = 0, AH_O = 0;

// Battery Discharge Termination Pin Mapping
const int TER_R = 9, TER_Y = 10, TER_G = 11, TER_O = 12;
bool Rdischarge = false, Ydischarge = false, Gdischarge = false, Odischarge = false;

void START_DIS(int DISCHARGE_ENABLE_PIN) {
  digitalWrite(DISCHARGE_ENABLE_PIN, LOW );
}

void END_DIS(int DISCHARGE_ENABLE_PIN) {
  digitalWrite(DISCHARGE_ENABLE_PIN, HIGH);
}

void DISABLE_ALL_DISCHARGE() {
  Serial.println();
  Serial.println("Terminating all Discharges");
  Rdischarge = false, Ydischarge = false, Gdischarge = false, Odischarge = false;
  END_DIS(TER_R); END_DIS(TER_Y); END_DIS(TER_O); END_DIS(TER_G);
  delay(100);
}

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
  BAT_VR = GetBATVoltage(VR); delay(100); //Serial.print("BAT Voltage R :"); Serial.println(BAT_VR);
  BAT_VY = GetBATVoltage(VY); delay(100); //Serial.print("BAT Voltage Y :"); Serial.println(BAT_VY);
  BAT_VG = GetBATVoltage(VG); delay(100); //Serial.print("BAT Voltage G :"); Serial.println(BAT_VG);
  BAT_VO = GetBATVoltage(VO); delay(100); //Serial.print("BAT Voltage O :"); Serial.println(BAT_VO);
  Serial.println();
}

//Push Button for starting and stopping discharge
const int PushbuttonPin = 2;
int PushbuttonState = 0;

uint32_t TIM_IN_R = 0, TIM_IN_Y = 0, TIM_IN_G = 0, TIM_IN_O = 0;

void ButtonPressed() {
  //Initiate discharge at begining only
  //  if (discharge == false && AH_R == 0 && AH_Y == 0 && AH_G == 0 && AH_O == 0) {
  RefreshBATVoltage();
  //Recording initial millis
  /*
    BAT_TR = millis();
    BAT_TY = millis();
    BAT_TG = millis();
  */

  //Stopping all on going discharge when PB is pressed and wait till next press
  if (Odischarge == true || Gdischarge == true || Ydischarge == true || Rdischarge == true ) {
    DISABLE_ALL_DISCHARGE();
    Serial.println("EMERGENCY STOP VIA PB");
  }
  else {

    if (Odischarge == false && BAT_VO > 3.7) {
      // AH value preserved if available
      if ( AH_O == 0) {
        Serial.println("STARTING FROM 0 AH");
        TIM_IN_O = millis();
      }
      else
      {
        Serial.println("OLD AH VALUE PRESERVED");
      }
      Odischarge = true;
    }
  }
}

void IncrementAh() {
  AH_R = AH_R + 1;
  AH_Y = AH_Y + 1;
  AH_G = AH_G + 1;
  AH_O = AH_O + 1;

  /*
    Serial.print(" AHR=");
    Serial.print(AH_R);
    Serial.print(" AHY=");
    Serial.print(AH_Y);
    Serial.print(" AHG=");
    Serial.print(AH_G);
    Serial.print(" AHO=");
    Serial.println(AH_O);
  */
}

void setup() {
  //Timer for incrementing AH per second
  //int tickEvent = t.every(1000, IncrementAh);


  // Pin Mode configuration for Pushbutton Input with Internal Pullup
  pinMode(PushbuttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PushbuttonPin), ButtonPressed, FALLING);

  // Pin Mode configuration for Discharge Termination Pins
  pinMode(TER_R, OUTPUT);
  pinMode(TER_Y, OUTPUT);
  pinMode(TER_G, OUTPUT);
  pinMode(TER_O, OUTPUT);

  // Print Code Revision to the LCD.
  Serial.begin(9600);
  Serial.println("Battery Checker");
  lcd.begin(16, 2);
  lcd.print("Battery Checker");
  Serial.println("R 1.0");
  lcd.setCursor(0, 1);
  lcd.print("R 1.0");
  DISABLE_ALL_DISCHARGE();
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("Push to Start");
}

void loop() {
  t.update();// Timer for AH calc
  RefreshBATVoltage();

  //Printing Capacity on LCD
  lcd.clear();

  Serial.print("BAT Voltage O :"); Serial.println(BAT_VO);
  Serial.print("Odischarge"); Serial.println(Odischarge);
  lcd.setCursor(0, 0);
  lcd.print("O= ");


  if ( BAT_VO >= 3.0 && Odischarge == true) {
    AH_O = ((millis() - TIM_IN_O) * 0.001) / 3600; //in AH
    Serial.println("Discharging Orange Battery");

  }
  else
  {
    Serial.println("NOT -Discharging Orange Battery");
    Odischarge = false;
    END_DIS(TER_O);
  }
  lcd.print(AH_O);

  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("O= ");
  lcd.print(BAT_VO);
  delay(500);

}
/*

  lcd.setCursor(0, 0);
  lcd.print(((millis() - TIM_IN_R) * 0.001) / 3600);
  Serial.println((millis() - ts1) * 0.001);
  }




  lcd.print(AH_R / 3600);
  lcd.setCursor(8, 0);
  lcd.print("Y= ");
  lcd.print(AH_Y / 3600);
  lcd.setCursor(0, 1);
  lcd.print("G= ");
  lcd.print(AH_G / 3600);
  lcd.setCursor(8, 1);
  lcd.print("O= ");
  lcd.print(AH_O / 3600);
  }

  //RefreshBATVoltage();
  /*
  if ( BAT_VR < 3.0 ) {
    END_DIS(TER_R);
    lcd.setCursor(0, 0);
    lcd.print("BAT RED DONE");
    Serial.println("BAT RED DONE");
  }

  if ( BAT_VG < 3.0 ) {
    END_DIS(TER_G);
    lcd.setCursor(0, 0);
    lcd.print("BAT GRE DONE");
    Serial.println("BAT GRE DONE");
  }

  if ( BAT_VY < 3.0 ) {
    END_DIS(TER_Y);
    lcd.setCursor(0, 0);
    lcd.print("BAT YEL DONE");
    Serial.println("BAT YEL DONE");
  }

  //DISABLE_DISCHARGE();
  }
*/
