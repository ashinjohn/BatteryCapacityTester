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
float BatteryCheckVolt = 3.0; // Reduced from 3.7 to 3.0 to reduce failure
const int COUNT_TH = 5;
int COUNT_R = 0, COUNT_Y = 0, COUNT_G = 0, COUNT_O = 0;

// Battery Discharge Termination Pin Mapping
const int TER_R = 12, TER_Y = 11, TER_G = 9, TER_O = 10;
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
volatile float voltarray[20];
int voltarraysize = 20;

void sort(float voltarray[], int voltarraysize) {
  for (int i = 0; i < (voltarraysize - 1); i++) {
    for (int o = 0; o < (voltarraysize - (i + 1)); o++) {
      if (voltarray[o] > voltarray[o + 1]) {
        float t = voltarray[o];
        voltarray[o] = voltarray[o + 1];
        voltarray[o + 1] = t;
      }
    }
  }
}

float GetBATVoltage(int Battery)
{
  for (int i = 0; i < (voltarraysize - 1); i++) {
    delay(10);
    voltarray[i] = analogRead(Battery) * (5.0 / 1024.0);
  }
  sort(voltarray, voltarraysize);
  return voltarray[10];
  //return (analogRead(Battery) * (5.0 / 1024.0));
}

void RefreshBATVoltage()
{
  Serial.println();
  Serial.println("Battery Voltages");
  BAT_VR = GetBATVoltage(VR);  Serial.print("BAT Voltage R :"); Serial.println(BAT_VR);
  BAT_VY = GetBATVoltage(VY);  Serial.print("BAT Voltage Y :"); Serial.println(BAT_VY);
  BAT_VG = GetBATVoltage(VG);  Serial.print("BAT Voltage G :"); Serial.println(BAT_VG);
  BAT_VO = GetBATVoltage(VO);  Serial.print("BAT Voltage O :"); Serial.println(BAT_VO);
  Serial.println();
}

//Push Button for starting and stopping discharge
const int PushbuttonPin = 2;
int PushbuttonState = 0;

uint32_t TIM_IN_R = 0, TIM_IN_Y = 0, TIM_IN_G = 0, TIM_IN_O = 0;

void ButtonPressed() {
  lcd.begin(16, 2); // To test fix loss of display while connecting or disconnecting battery
  RefreshBATVoltage();

  //Stopping all on going discharge when PB is pressed and wait till next press
  if (Odischarge == true || Gdischarge == true || Ydischarge == true || Rdischarge == true ) {
    DISABLE_ALL_DISCHARGE();
    Serial.println("EMERGENCY STOP VIA PB");
  }
  else {

    //*************** ORANGE **********************
    if (Odischarge == false && BAT_VO > BatteryCheckVolt) {
      // AH value preserved if available
      if ( AH_O == 0) {
        Serial.println("STARTING ORANGE FROM 0 AH");
        TIM_IN_O = millis();
      }
      else
      {
        Serial.println("OLD AH VALUE ORANGE PRESERVED");
      }
      Odischarge = true;
      START_DIS(TER_O); delay(500);
    }
    else {
      Serial.println("ORANGE BATTERY VOLTAGE LOW");
    }
  }

  //*************** GREEN **********************
  if (Gdischarge == false && BAT_VG > BatteryCheckVolt) {
    // AH value preserved if available
    if ( AH_G == 0) {
      Serial.println("STARTING GREEN FROM G AH");
      TIM_IN_G = millis();
    }
    else
    {
      Serial.println("OLD AH VALUE GREEN PRESERVED");
    }
    Gdischarge = true;
    START_DIS(TER_G); delay(500);
  }
  else {
    Serial.println("GREEN BATTERY VOLTAGE LOW");
  }

  //*************** YELLOW **********************
  if (Ydischarge == false && BAT_VY > BatteryCheckVolt) {
    // AH value preserved if available
    if ( AH_Y == 0) {
      Serial.println("STARTING YELLOW FROM G AH");
      TIM_IN_Y = millis();
    }
    else
    {
      Serial.println("OLD AH VALUE YELLOW PRESERVED");
    }
    Ydischarge = true;
    START_DIS(TER_Y); delay(500);
  }
  else {
    Serial.println("YELLOW BATTERY VOLTAGE LOW");
  }

  //*************** RED **********************
  if (Rdischarge == false && BAT_VR > BatteryCheckVolt) {
    // AH value preserved if available
    if ( AH_R == 0) {
      Serial.println("STARTING RED FROM G AH");
      TIM_IN_R = millis();
    }
    else
    {
      Serial.println("OLD AH VALUE RED PRESERVED");
    }
    Rdischarge = true;
    START_DIS(TER_R); delay(500);
  }
  else {
    Serial.println("YELLOW BATTERY VOLTAGE LOW");
  }

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
  delay(500);

  //Printing Capacity on LCD
  lcd.clear();

  //******************** ORGANGE **************************
  Serial.print("BAT Voltage O :"); Serial.println(BAT_VO);
  Serial.print("ORA discharge :"); Serial.println(Odischarge);
  lcd.setCursor(0, 0);

  if ( BAT_VO >= 3.0 && Odischarge == true) {
    AH_O = ((millis() - TIM_IN_O) * 0.001) / 3600; //in AH
    Serial.println("Discharging Orange Battery");
    lcd.print("O= ");
    COUNT_O = COUNT_TH ;
  }
  else if (COUNT_O <= 0)
  {
    lcd.print("O? ");
    Serial.println("NOT -Discharging Orange Battery");
    Odischarge = false;
    END_DIS(TER_O);
  }
  else {
    COUNT_O--;
    lcd.print("O! ");
    Serial.println("Reducing Orange Low Counter");
  }
  lcd.print(AH_O);

  //******************** YELLOW **************************
  Serial.print("BAT Voltage Y :"); Serial.println(BAT_VY);
  Serial.print("YEL discharge :"); Serial.println(Ydischarge);
  lcd.setCursor(8, 0);

  if ( BAT_VY >= 3.0 && Ydischarge == true) {
    AH_Y = ((millis() - TIM_IN_Y) * 0.001) / 3600; //in AH
    Serial.println("Discharging Yellow Battery");
    lcd.print("Y= ");
    COUNT_Y = COUNT_TH;
  }
  else if (COUNT_Y <= 0)
  {
    lcd.print("Y? ");
    Serial.println("NOT -Discharging Yellow Battery");
    Ydischarge = false;
    END_DIS(TER_Y);
  }
  else {
    COUNT_Y--;
    lcd.print("Y! ");
    Serial.println("Reducing Yellow Low Counter");
  }
  lcd.print(AH_Y);

  //******************** GREEN **************************
  Serial.print("BAT Voltage G :"); Serial.println(BAT_VG);
  Serial.print("GRE discharge :"); Serial.println(Gdischarge);
  lcd.setCursor(0, 1);

  if ( BAT_VG >= 3.0 && Gdischarge == true) {
    AH_G = ((millis() - TIM_IN_G) * 0.001) / 3600; //in AH
    Serial.println("Discharging Green Battery");
    lcd.print("G= ");
    COUNT_G = COUNT_TH;
  }
  else if (COUNT_G <= 0)
  {
    lcd.print("G? ");
    Serial.println("NOT -Discharging Green Battery");
    Gdischarge = false;
    END_DIS(TER_G);
  }
  else {
    COUNT_G--;
    lcd.print("G! ");
    Serial.println("Reducing Green Low Counter");
  }
  lcd.print(AH_G);

  //******************** RED **************************
  Serial.print("BAT Voltage R :"); Serial.println(BAT_VR);
  Serial.print("GRE discharge :"); Serial.println(Rdischarge);
  lcd.setCursor(8, 1);

  if ( BAT_VR >= 3.0 && Rdischarge == true) {
    AH_R = ((millis() - TIM_IN_R) * 0.001) / 3600; //in AH
    Serial.println("Discharging Red Battery");
    lcd.print("R= ");
    COUNT_R = COUNT_TH;
  }
  else if (COUNT_R <= 0)
  {
    lcd.print("R? ");
    Serial.println("NOT -Discharging Red Battery");
    Rdischarge = false;
    END_DIS(TER_R);
  }
  else {
    COUNT_R--;
    lcd.print("R! ");
    Serial.println("Reducing Red Low Counter");
  }
  lcd.print(AH_R);

  delay(500);
  //Printing Voltage on LCD
  lcd.clear();
  //******************** Orange **************************

  lcd.setCursor(0, 0);
  lcd.print("O= ");
  lcd.print(BAT_VO);

  //******************** Yellow **************************

  lcd.setCursor(8, 0);
  lcd.print("Y= ");
  lcd.print(BAT_VY);

  //******************** Green **************************

  lcd.setCursor(0, 1);
  lcd.print("G= ");
  lcd.print(BAT_VG);

  //******************** Red **************************

  lcd.setCursor(8, 1);
  lcd.print("R= ");
  lcd.print(BAT_VR);

}
