/*
Simple MF52D analog temperature sensor example sketch.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

Based on part: NTC Thermistor Temperature Sensor 10K OHM MF52-103 3435 1%

https://forum.arduino.cc/index.php?topic=614371.0
*/

/*
Assumed MF52D temperature sensor design:
 - The balance resistor is R1.
 - The MF52D resistance that varies is R2.
 - MF52D characteristic: Increasing temperature => decreasing resistance.

 AVRef ----
 (5V)     |
         ---
         |R| R1 = 10K Resistor
         ---
          |----- vOut --> Arduino Pin A3 (MF52DPIN)
         ---
         |R| R2 = MF52D Thermistor
         ---
          |
 Gnd ------
*/

#include <AlogTSensors.h>

#define MF52DPIN A3                         //The vDivider class default is A0.

#define R1 10000                            //The vDivider class default is 10K.

#define MF52DSAMPLES 32                     //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAY 1                       //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.

#define MF52DCBETA 3435                     //The MF52D class default is 3435.
#define MF52DNOMRST 10000                   //The MF52D class default is 10K.
#define MF52DNOMTEMP 25.0                   //The MF52D class default is 25.0degC.

#define DELAY 5000

MF52D myMF52D(MF52DPIN, R1);                //The balance resistor is R1.

void setup() {
  //Start the serial console.
  Serial.begin(9600);
  //Set up is only needed if the defaults need to be changed.
  //  Defaults: samples = 16, sample delay = 1, avRef = 5.0, adcMax = 1023
  myMF52D.setConsts(MF52DSAMPLES, SAMPLEDELAY, AVREF, ADCMAXVALUE);
  //  Defaults: beta = 3435, nominal resistance = 10K, nominal temperature = 25.0degC
  myMF52D.setCBeta(MF52DCBETA, MF52DNOMRST, MF52DNOMTEMP);
}

void loop() {
  byte myMF52DPin = myMF52D.analogPin;
  //An int is acceptable, but an unsigned int is better.
  unsigned int avgADC = myMF52D.readADC();                    //Get a new reading, 32 samples, 1ms apart, and then averaged.
  float vOut = myMF52D.calcVOut(avgADC);                      //Pass the MF52D ADC reading already taken, else this function will get a new reading.
  //Calculate the MF52D resistance using the ADC reading directly.  
  float myMF52DR2i = myMF52D.calcR2(avgADC);                  //Pass the MF52D ADC reading already taken, else this function will get a new reading.
  //Calculate the MF52D resistance using the calculated voltage.
  float myMF52DR2f = myMF52D.calcR2(vOut);                    //Pass the MF52D voltage already calculated, else this function will get a new reading.
  float temperatureCi = myMF52D.readTemperatureC(myMF52DR2i); //Pass the MF52D i resistance already calculated, else this function will get a new reading.
  float temperatureCf = myMF52D.readTemperatureC(myMF52DR2f); //Pass the MF52D f resistance already calculated, else this function will get a new reading.
  Serial.print("MF52D Sensor on Arduino pin: A");
  Serial.println(myMF52DPin - 14);
  Serial.print("  Avg ADC Val   = ");
  Serial.println(avgADC);
  Serial.print("  Voltage Out   = ");
  Serial.print(vOut);
  Serial.println("V");
  Serial.print("  Resistance i  = ");
  Serial.print(myMF52DR2i);
  Serial.println("R");
  Serial.print("  Resistance f  = ");
  Serial.print(myMF52DR2f);
  Serial.println("R");
  Serial.print("  Temperature i = ");
  Serial.print(temperatureCi);
  Serial.print("\xC2\xB0");
  Serial.println("C");
  Serial.print("  Temperature f = ");
  Serial.print(temperatureCf);
  Serial.print("\xC2\xB0");
  Serial.println("C");
  Serial.println();
  //Wait for a while...
  delay(DELAY);
}

//EOF
