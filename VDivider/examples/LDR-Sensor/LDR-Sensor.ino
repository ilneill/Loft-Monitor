/*
An LDR (Voltage Divider) example sketch.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://en.wikipedia.org/wiki/Photoresistor
https://en.wikipedia.org/wiki/Voltage_divider
*/

/*
Assumed LDR sensor design:
 - The LDR resistance that varies is R1.
 - The balance resistor is R2.
 - LDR characteristic: Increasing light => decreasing resistance.

 AVRef ----
 (5V)     |
         ---
         |R| R1 = LDR (e.g. GL5516, min 100R, max 1.5MR+)
         ---
          |----- vOut --> Arduino Pin A5 (LDRPIN)
         ---
         |R| R2 = 100K Resistor
         ---
          |
 Gnd ------
*/

#include <VDivider.h>

#define LDRPIN A5                           //The vDivider class default is A0.
#define R2 100000                           //The vDivider class default is 10K.

#define LDRSAMPLES 32                       //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAY 5                       //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.  

#define DELAY 5000

vDivider myLDR(LDRPIN, R2, false);          //The balance resistor is R2 (not R1).

void setup() {
  //Start the serial output at 9600 baud.
  Serial.begin(9600);
  //Set up voltage divider constants, if necessary.
  myLDR.setConsts(LDRSAMPLES, SAMPLEDELAY, AVREF, ADCMAXVALUE);
}

void loop() {
  byte myLDRPin = myLDR.analogPin;
  //An int is acceptable, but an unsigned int is better.
  unsigned int avgLDR = myLDR.readADC();    //Get a new reading, 32 samples, 5ms apart, and then averaged.
  float vLDR = myLDR.calcVOut(avgLDR);      //Pass the LDR ADC reading already taken, else this function will get a new reading.
  float rLDRi = myLDR.calcR1(avgLDR);       //Pass the LDR ADC reading already taken, else this function will get a new reading.
  float rLDRf = myLDR.calcR1(vLDR);         //Pass the LDR voltage already calculated, else this function will get a new reading.
  Serial.print("LDR Sensor on Arduino pin: A");
  Serial.println(myLDRPin - 14);
  Serial.print("  Avg ADC Val  = ");
  Serial.println(avgLDR);
  Serial.print("  Voltage Out  = ");
  Serial.print(vLDR);
  Serial.println("V");
  Serial.print("  Resistance i = ");
  Serial.print(rLDRi);
  Serial.println("R");
  Serial.print("  Resistance f = ");
  Serial.print(rLDRf);
  Serial.println("R");
  Serial.println();  
  //Wait for a while...
  delay(DELAY);
}

//EOF
