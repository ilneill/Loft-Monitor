/*
Another Voltage Divider example sketch.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://en.wikipedia.org/wiki/Voltage_divider
*/

/*
Assumed Voltage Divider A design:
 - The balance resistor is R1.
 - The resistance that varies is R2.

 AVRef ----
 (5V)     |
         ---
         |R| R1 = 10K Resistor
         ---
          |----- vOut --> Arduino Pin A3 (VDAPIN)
         ---
         |R| R2 = Potentiometer
         ---
          |
 Gnd ------

Assumed Voltage Divider B design:
 - The balance resistor is R2.
 - The resistance that varies is R1.

 AVRef ----
 (5V)     |
         ---
         |R| R1 = Potentiometer
         ---
          |----- vOut --> Arduino Pin A4 (VDBPIN)
         ---
         |R| R2 = 100K Resistor
         ---
          |
 Gnd ------
*/

#include <VDivider.h>

#define VDAPIN A3                           //The vDivider class default is A0.
#define VDBPIN A4                           //The vDivider class default is A0.
#define R1A 10000                           //The vDivider class default is 10K.
#define R2B 100000                          //The vDivider class default is 10K.

#define VDIVSAMPLESA 8                      //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAYA 1                      //The vDivider class default is 1ms delay between samples.
#define VDIVSAMPLESB 24                     //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAYB 2                      //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.  

#define DELAY 5000

vDivider myDividerA(VDAPIN, R1A, true);     //The balance resistor is R1.
vDivider myDividerB(VDBPIN, R2B, false);    //The balance resistor is R2 (not R1).

void setup() {
  //Start the serial output at 9600 baud.
  Serial.begin(9600);
  //Set up voltage divider constants, if necessary.
  myDividerA.setConsts(VDIVSAMPLESA, SAMPLEDELAYA, AVREF, ADCMAXVALUE);
  myDividerB.setConsts(VDIVSAMPLESB, SAMPLEDELAYB, AVREF, ADCMAXVALUE);
  //Confirm/Discover how many vDividers have been defined.
  Serial.print("Number of defined Voltage Dividers: ");
  Serial.println(vDivider::vDivCount());
  Serial.println();
}

void loop() {
  byte dividerPinA = myDividerA.analogPin;
  byte dividerPinB = myDividerB.analogPin;
  float vOutA = myDividerA.calcVOut();      //Get a new reading, 8 samples, 1ms apart, averaged, and then calculate the voltage.
  float vOutB = myDividerB.calcVOut();      //Get a new reading, 24 samples, 2ms apart averaged, and then calculate the voltage.
  float potValA = myDividerA.calcR2(vOutA); //Pass the voltage already calculated, else this function will get a new reading.
  float potValB = myDividerB.calcR1(vOutB); //Pass the voltage already calculated, else this function will get a new reading.
  Serial.print("Voltage Divider on Arduino pin: A");
  Serial.println(dividerPinA - 14);
  Serial.print("  Voltage Out    = ");
  Serial.print(vOutA);
  Serial.println("V");
  Serial.print("  Pot Resistance = ");
  Serial.print(potValA);
  Serial.println("R");
  Serial.println();
  Serial.print("Voltage Divider on Arduino pin: A");
  Serial.println(dividerPinB - 14);
  Serial.print("  Voltage Out    = ");
  Serial.print(vOutB);
  Serial.println("V");
  Serial.print("  Pot Resistance = ");
  Serial.print(potValB);
  Serial.println("R");
  Serial.println();
  //Wait for a while...
  delay(DELAY);
}

//EOF
