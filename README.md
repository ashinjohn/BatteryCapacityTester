# **Battery Capacity Checker**

Estimates the capacity of batteries, Firmware fine tuned for 18650 Lithium Battery
4 channels supporting 30V or 3A ( Not both at a time, Heat dissipation issues)
IDE - Arduino
MCU - ATmega328P Barebone Arduino

## **Hardware Design**
1.Dummy Load Circuit
2.Battery Voltage Mesurement using internal ADC
3.Cutoff of Dummy Load when battery voltage reaches the lower thershold.
4.HD44780 16x2 LCD Module
5.Push Button Potentiometer ( Pending )

## **Firmware**
1.Voltage Measurement using internal ADC
2.Software Timer for computing time elapsed
3.Discharge Termination when battery voltage reaches the lower thershold.
4.HD44780 16x2 LCD Module
5.User Interface using Push Button Potentiometer ( Pending )
6.Serial Interface for configuring parameters 
