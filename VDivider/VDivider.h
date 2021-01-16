/*
Arduino library for Voltage Dividers.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://arduino.github.io/arduino-cli/library-specification/
https://roboticsbackend.com/arduino-create-library/
https://roboticsbackend.com/arduino-object-oriented-programming-oop/

https://en.wikipedia.org/wiki/Voltage_divider
https://forum.arduino.cc/index.php?topic=303189.0
https://forum.allaboutcircuits.com/threads/why-adc-1024-is-correct-and-adc-1023-is-just-plain-wrong.80018/
https://forums.adafruit.com/viewtopic.php?f=25&t=11597&sid=9f283ac80d4fd357991febd2019ff8a8#p55161
*/

/*
Assumed Voltage Divider design:
 - The known balance resistor can be either R1 or R2.
 - The resistance that is unknown or varies can be either R2 or R1.

 AVRef ----
          |
         ---
         |R| R1 (default, known, balance resistor)
         ---
          |----- VOut --> Arduino Pin A0
         ---
         |R| R2 (default, unknown or variable resistor)
         ---
          |
 Gnd ------
*/

#ifndef VDIVIDER_H
  #define VDIVIDER_H

  #include <Arduino.h>

  #define _ANALOGPIN A0
  #define _BALANCERESISTOR 10000.0  //Ohms.
  #define _VDIVSAMPLES 16           //16bit counter.
  #define _SAMPLEDELAY 1            //Milliseconds, 0 - 255.

  #define _ADCREADYDELAY 10         //Milliseconds, 0 - 255.

  //Platform dependant default analogue reference voltage.
  #ifndef __AVR__
    #define _AVREF 3.3
  #else
    #define _AVREF 5.0
  #endif

  #define _ADCMAX 1023              //The maximum value a 10bit ADC can return, 1 is added to get the ADC steps when needed.

  class vDivider {
  public:
    vDivider(uint8_t pin = _ANALOGPIN, float balanceResistor = _BALANCERESISTOR, bool isR1 = true);
    ~vDivider();
    void setConsts(uint16_t samples = _VDIVSAMPLES, uint8_t sampleDelay = _SAMPLEDELAY, uint8_t adcReadyDelay = _ADCREADYDELAY, float avRef = _AVREF, uint16_t adcMax = _ADCMAX);
    uint8_t analogPin;          //The pin that the voltage divider is connected to.
    static uint8_t vDivCount(); //A function to return the number of defined voltage dividers.
    uint16_t readADC();         //A function to read the ADC value from the divider pin.
    float calcVOut();           //Calculate the divider voltage by reading the ADC value.
    float calcVOut(uint16_t);   //Function version that will use a given ADC value.
    float calcR1();             //Calculate the R1 resistance by reading the ADC value.
    float calcR1(uint16_t);     //Function version that will use a given ADC reading.
    float calcR1(int);          //Function version that will use a given ADC reading (accepting it as an int).
    float calcR1(float);        //Function version that will use a given divider voltage.
    float calcR2();             //Calculate the R2 resistance by reading the ADC value.
    float calcR2(uint16_t);     //Function version that will use a given ADC reading.
    float calcR2(int);          //Function version that will use a given ADC reading (accepting it as an int).
    float calcR2(float);        //Function version that will use a given divider voltage.
    float calcR1x();            //Calculate the R1 resistance by reading the ADC value, assuming the balance resistor is R2.
    float calcR1x(uint16_t);    //Function version that will use a given ADC reading.
    float calcR1x(int);         //Function version that will use a given ADC reading (accepting it as an int).
    float calcR1x(float);       //Function version that will use a given divider voltage.
    float calcR2x();            //Calculate the R2 resistance by reading the ADC value, assuming the balance resistor is R1.
    float calcR2x(uint16_t);    //Function version that will use a given ADC reading.
    float calcR2x(int);         //Function version that will use a given ADC reading (accepting it as an int).
    float calcR2x(float);       //Function version that will use a given divider voltage.

  protected:
    uint8_t _pin;
    float _balanceResistor;
    bool _isR1;
    uint16_t _samples;
    uint8_t _sampleDelay;
    uint8_t _adcReadyDelay;
    float _avRef;
    uint16_t _adcMax;

  private:
    static uint8_t _vDivCounter;
    void init();
    float doCalcR1(uint16_t);   //Calculate R1 using the provided ADC reading.
    float doCalcR1(float);      //Function version that will use a given divider voltage.
    float doCalcR2(uint16_t);   //Calculate R2 using the provided ADC reading.
    float doCalcR2(float);      //Function version that will use a given divider voltage.
    float doCalcR1x(uint16_t);  //Calculate R1 using the provided ADC reading, assuming the balance resistor is R2.
    float doCalcR1x(float);     //Function version that will use a given divider voltage.
    float doCalcR2x(uint16_t);  //Calculate R2 using the provided ADC reading, assuming the balance resistor is R1.
    float doCalcR2x(float);     //Function version that will use a given divider voltage.
    float calcRRatio(uint16_t); //Calculate the divider resistor ratio using the provided ADC reading.
    float calcRRatio(float);    //Function version that will use a given divider voltage.
  };
#endif

//EOF
