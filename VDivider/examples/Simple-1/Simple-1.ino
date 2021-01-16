/*
Simple Voltage Divider example sketch 1.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://en.wikipedia.org/wiki/Voltage_divider
*/

/*
Assumed Voltage Divider design:
 - The balance resistor is R1.
 - The resistance that varies is R2.

 AVRef ----
 (5V)     |
         ---
         |R| R1 = 10K Resistor
         ---
          |----- vOut --> Arduino Pin A1
         ---
         |R| R2 = Potentiometer
         ---
          |
 Gnd ------
*/

#include <VDivider.h>

#define VDPIN A1                            //The vDivider class default is A0.
#define R1 10000                            //The vDivider class default is 10K.

#define VDIVSAMPLES 8                       //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAY 1                       //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.  

#define DELAY 5000

vDivider myDivider(VDPIN, R1, true);        //The balance resistor is R1.

void setup() {
  //Start the serial output at 9600 baud.
  Serial.begin(9600);
  //Set up voltage divider constants, if necessary.
  myDivider.setConsts(VDIVSAMPLES, SAMPLEDELAY, AVREF, ADCMAXVALUE);
}

void loop() {
  byte dividerPin = myDivider.analogPin;
  float potVal = myDivider.calcR2();        //Get a new reading, 8 samples, 1ms apart, averaged, and then calculate R2.
  Serial.print("Voltage Divider on Arduino pin: A");
  Serial.println(dividerPin - 14);
  Serial.print("  Pot Resistance = ");
  Serial.print(potVal);
  Serial.println("R");
  Serial.println();
  //Wait for a while...
  delay(DELAY);
}

//EOF
