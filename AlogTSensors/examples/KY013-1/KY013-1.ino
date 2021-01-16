/*
Simple KY013 analog temperature sensor example sketch 1.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

Based on part: AZ Deliveries 100K@T25 KY013 module.

https://arduinomodules.info/ky-013-analog-temperature-sensor-module/
http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
*/

/*
Assumed KY013 temperature sensor design:
 - The balance resistor is R1a + R1b.
 - The KY013 resistance that varies is R2.
 - KY013 characteristic: Increasing temperature => decreasing resistance.

 AVRef ----
 (5V)     |
         ---
         |R| R1a = 100K Resistor (external)
         ---
          |
         ---
         |R| R1b = 10K Resistor (on-board)
         ---
          |----- vOut --> Arduino Pin A2 (KY013PIN)
         ---
         |R| R2 = KY013 Thermistor
         ---
          |
 Gnd ------
*/

#include <AlogTSensors.h>

#define KY013PIN A2                         //The vDivider class default is A0.

#define R1 110000                           //The vDivider class default is 10K.

#define KY013SAMPLES 32                     //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAY 4                       //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.

//NTC Thermistor Temperature Sensor AZ Deliveries 10K@T25 KY013 module (from ebook datasheet).
//#define KY013COEFFICIENT1 1.009249522e-03 //0.001009249522.
//#define KY013COEFFICIENT2 2.378405444e-04 //0.0002378405444.
//#define KY013COEFFICIENT3 2.019202697e-07 //0.0000002019202697.

//NTC Thermistor Temperature Sensor AZ Deliveries 100K@T25 KY013 module (calculated with NTCcalculator).
#define KY013COEFFICIENT1 5.182977433e-04   //0.0005182977433.
#define KY013COEFFICIENT2 2.252079282e-04   //0.0002252079282.
#define KY013COEFFICIENT3 1.615362158e-07   //0.0000001615362158.

#define DELAY 5000

KY013 myKY013(KY013PIN, R1);                //The balance resistor is R1.

void setup() {
  //Start the serial console.
  Serial.begin(9600);
  //Init is only needed if the defaults need to be changed.
  //  Defaults: samples = 16, sample delay = 1, avRef = 5.0, adcMax = 1023
  myKY013.setConsts(KY013SAMPLES, SAMPLEDELAY, AVREF, ADCMAXVALUE);
  //  Defaults: coef1 = 0.0005182977433, coef2 = 0.0002252079282, coef3 = 0.0000001615362158
  myKY013.setC123(KY013COEFFICIENT1, KY013COEFFICIENT2, KY013COEFFICIENT3);
}

void loop() {
  byte myKY013Pin = myKY013.analogPin;
  //An int is acceptable, but an unsigned int is better.
  unsigned int avgADC = myKY013.readADC();                    //Get a new reading, 32 samples, 4ms apart, and then averaged.
  float vOut = myKY013.calcVOut(avgADC);                      //Pass the KY013 ADC reading already taken, else this function will get a new reading.
  //Calculate the KY013 resistance using the ADC reading directly.  
  float myKY013R2i = myKY013.calcR2(avgADC);                  //Pass the KY013 ADC reading already taken, else this function will get a new reading.
  //Calculate the KY013 resistance using the calculated voltage.
  float myKY013R2f = myKY013.calcR2(vOut);                    //Pass the KY013 voltage already calculated, else this function will get a new reading.
  float temperatureCi = myKY013.readTemperatureC(myKY013R2i); //Pass the KY013 i resistance already calculated, else this function will get a new reading.
  float temperatureCf = myKY013.readTemperatureC(myKY013R2f); //Pass the KY013 f resistance already calculated, else this function will get a new reading.
  Serial.print("KY013 Sensor on Arduino pin: A");
  Serial.println(myKY013Pin - 14);
  Serial.print("  Avg ADC Val   = ");
  Serial.println(avgADC);
  Serial.print("  Voltage Out   = ");
  Serial.print(vOut);
  Serial.println("V");
  Serial.print("  Resistance i  = ");
  Serial.print(myKY013R2i);
  Serial.println("R");
  Serial.print("  Resistance f  = ");
  Serial.print(myKY013R2f);
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
