/*
Simple TMP36 analog temperature sensor example sketch.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://bc-robotics.com/tutorials/using-a-tmp36-temperature-sensor-with-arduino/
*/

/*
Assumed TMP36 temperature sensor design:

 Gnd ------
          |
   ----   |
  / T |----
 |  M |
 |  P |--------- vOut --> Arduino Pin A1 (TMPPIN)
 |  3 |
  \ 6 |----
   ----   |
          |
 AVRef ----
 (5V)
*/

#include <AlogTSensors.h>

#define TMPPIN A1                           //The vDivider class default is A0.

#define TMP36SAMPLES 8                      //The vDivider class default is 16 samples per reading, averaged.
#define SAMPLEDELAY 2                       //The vDivider class default is 1ms delay between samples.
#define AVREF 5.0                           //This is the vDivider class default for most Arduinos.
#define ADCMAXVALUE 1023                    //The vDivider class default is 1023.  

#define TMP36OFFSET 0.5                     //The TMP36 class default is 0.5.
#define TMP36MULTIPLIER 100.0               //The TMP36 class default is 100.0.

#define DELAY 5000

TMP36 myTMP36(TMPPIN);

void setup() {
  //Start the serial console.
  Serial.begin(9600);
  //Set up is only needed if the defaults need to be changed.
  //  Defaults: samples = 16, sample delay = 1, avRef = 5.0, adcMax = 1023
  myTMP36.setConsts(TMP36SAMPLES, SAMPLEDELAY, AVREF, ADCMAXVALUE);
  //  Defaults: offset = 0.5, multiplier = 100.0
  myTMP36.setParms(TMP36OFFSET, TMP36MULTIPLIER);
}

void loop() {
  byte myTMP36Pin = myTMP36.analogPin;
  //An int is acceptable, but an unsigned int is better.
  unsigned int avgADC = myTMP36.readADC();              //Get a new reading, 32 samples, 2ms apart, and then averaged.
  float vOut = myTMP36.calcVOut(avgADC);                //Pass the TMP36 ADC reading already taken, else this function will get a new reading.
  float temperatureC = myTMP36.readTemperatureC(vOut);  //Pass the TMP36 voltage already calculated, else this function will get a new reading.
  Serial.print("TMP36 Sensor on Arduino pin: A");
  Serial.println(myTMP36Pin - 14);
  Serial.print("  Avg ADC Val = ");
  Serial.println(avgADC);
  Serial.print("  Voltage Out = ");
  Serial.print(vOut);
  Serial.println("V");
  Serial.print("  Temperature = ");
  Serial.print(temperatureC);
  Serial.print("\xC2\xB0");
  Serial.println("C");
  Serial.println();
  //Wait for a while...
  delay(DELAY);
}

//EOF
