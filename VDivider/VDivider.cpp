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

#include "VDivider.h"

//Initialise the vDivider counter.
uint8_t vDivider::_vDivCounter = 0;

/*!
 *  @brief  Instantiates a new vDivider class.
 *  @param  pin
 *          IO pin number that the voltage divider is connected to.
 *  @param  balanceResistor
 *          The fixed, known, resistor in the voltage divider.
 *  @param  isR1
 *          True/False if the balance resistor is R1.
 */

vDivider::vDivider(uint8_t pin, float balanceResistor, bool isR1) {
  analogPin = _pin = pin;
  _balanceResistor = balanceResistor;
  _isR1 = isR1;
  _samples = _VDIVSAMPLES;            //Set to the default.
  _sampleDelay = _SAMPLEDELAY;        //Set to the default.
  _adcReadyDelay = _ADCREADYDELAY;    //Set to the default.
  _avRef = _AVREF;                    //Set to the default.
  _adcMax = _ADCMAX;                  //Set to the default.
  _vDivCounter++;                     //A vDivider has been created.
  init();
}

vDivider::~vDivider() {
  _vDivCounter--;                     //A vDivider has been destroyed.
}

void vDivider::init() {
  pinMode(_pin, INPUT);
}

uint8_t vDivider::vDivCount() {
  return _vDivCounter;
}

/*!
 *  @brief  Set some vDivider class constants if needed.
 *  @param  samples
 *          The number of sensor readings to collect and average.
 *  @param  sampleDelay
 *          The number of milliseconds between each sensor reading.
 *  @param  adcReadyDelay
 *          The number of milliseconds between connecting to the pin and taking the first sensor reading.
 *  @param  avRef
 *          The reference voltage to use in the ADC->Voltage calculations.
 *  @param  adcMax
 *          The maximum value that can be returned from the ADC.
 */

void vDivider::setConsts(uint16_t samples, uint8_t sampleDelay, uint8_t adcReadyDelay, float avRef, uint16_t adcMax) {
  _samples = samples;
  _sampleDelay = sampleDelay;
  _adcReadyDelay = adcReadyDelay;
  _avRef = avRef;
  _adcMax = adcMax;
}

uint16_t vDivider::readADC() {
  float totalADC = 0;
  //Wait for the multiplexed ADC to connect and become ready for accurate reading.
  if(_adcReadyDelay > 0) {
    analogRead(_pin);
    delay(_adcReadyDelay);
  }
  for (uint16_t counter = 0; counter < _samples; counter++) {
    //Accumulate the readings, adding 0.5 to average the rounding down caused by the ADC quantisation.
    totalADC += (analogRead(_pin) + 0.5);
    delay(_sampleDelay);  //Default 1ms delay between samples.
  }
  //Average the reading and add 0.5 so that the rounding is to the nearest whole number, up or down.
  uint16_t averageADC = (totalADC / _samples) + 0.5;
  return averageADC;
}

float vDivider::calcVOut() {
  uint16_t averageADC = readADC();
  float vOut = averageADC * (_avRef / (float)(_adcMax + 1)); //Divided by the number of quantisation levels.
  return vOut;
}

//Calculate the divider voltage using the given ADC reading.
float vDivider::calcVOut(uint16_t averageADC) {
  constrain(averageADC, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float vOut = averageADC * (_avRef / (float)(_adcMax + 1)); //Divided by the number of quantisation levels.
  return vOut;
}

float vDivider::calcR1() {
  //float vOut = calcVOut();
  uint16_t averageADC = readADC();
  //float r1 = doCalcR1(vOut);
  float r1 = doCalcR1(averageADC);
  return r1;
}

//Calculate the R1 value using the given ADC reading.
float vDivider::calcR1(uint16_t averageADC) {
  constrain(averageADC, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float r1 = doCalcR1(averageADC);
  return r1;
}

//Calculate the R1 value using the given ADC reading (accepting it as an int).
float vDivider::calcR1(int averageADC) {
  float r1 = calcR1((uint16_t)averageADC);  //Cast the int to a uint16_t.
  return r1;
}

//Calculate the R1 value using the given divider voltage.
float vDivider::calcR1(float vOut) {
  constrain(vOut, 0, _avRef); //Ensure that the passed voltage is within hardware bounds.
  float r1 = doCalcR1(vOut);
  return r1;
}

float vDivider::doCalcR1(uint16_t averageADC) {
  float r1 = 0.0;
  if (_balanceResistor > 0.0) {
    if (_isR1) {
      r1 = _balanceResistor;
    }
    else {
      float rRatio = calcRRatio(averageADC);
      r1 = _balanceResistor * rRatio;
    }
  }
  return r1;
}

float vDivider::doCalcR1(float vOut) {
  float r1 = 0.0;
  if (_balanceResistor > 0.0) {
    if (_isR1) {
      r1 = _balanceResistor;
    }
    else {
      float rRatio = calcRRatio(vOut);
      r1 = _balanceResistor * rRatio;
    }
  }
  return r1;
}

float vDivider::calcR2() {
  //float vOut = calcVOut();
  uint16_t averageADC = readADC();
  //float r2 = doCalcR2(vOut);
  float r2 = doCalcR2(averageADC);
  return r2;
}

//Calculate the R2 value using the given ADC reading.
float vDivider::calcR2(uint16_t averageADC) {
  constrain(averageADC, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float r2 = doCalcR2(averageADC);
  return r2;
}

//Calculate the R2 value using the given ADC reading.
float vDivider::calcR2(int averageADC) {
  float r2 = calcR2((uint16_t)averageADC);  //Cast the int to a uint16_t.
  return r2;
}

//Calculate the R2 value using the given divider voltage.
float vDivider::calcR2(float vOut) {
  constrain(vOut, 0, _avRef); //Ensure that the passed voltage is within hardware bounds.
  float r2 = doCalcR2(vOut);
  return r2;
}

float vDivider::doCalcR2(uint16_t averageADC) {
  float r2 = 0.0;
  if (_balanceResistor > 0.0) {
    if (_isR1) {
      float rRatio = calcRRatio(averageADC);
      r2 = _balanceResistor / rRatio;
    }
    else {
      r2 = _balanceResistor;
    }
  }
  return r2;
}

float vDivider::doCalcR2(float vOut) {
  float r2 = 0.0;
  if (_balanceResistor > 0.0) {
    if (_isR1) {
      float rRatio = calcRRatio(vOut);
      r2 = _balanceResistor / rRatio;
    }
    else {
      r2 = _balanceResistor;
    }
  }
  return r2;
}

//Calculate the R1 value assuming that the balance resistor is R2.
float vDivider::calcR1x() {
  //float vOut = calcVOut();
  uint16_t averageADC = readADC();
  //float r1 = doCalcR1x(vOut);
  float r1 = doCalcR1x(averageADC);
  return r1;
}

//Calculate the R1 value using the given ADC reading, and assuming that the balance resistor is R2.
float vDivider::calcR1x(uint16_t averageADC) {
  constrain(averageADC, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float r1 = doCalcR1x(averageADC);
  return r1;
}

//Calculate the R1 value using the given ADC reading, and assuming that the balance resistor is R2.
float vDivider::calcR1x(int averageADC) {
  float r1 = calcR1x((uint16_t)averageADC); //Cast the int to a uint16_t.
  return r1;
}

//Calculate the R1 value using the given divider voltage, and assuming that the balance resistor is R2.
float vDivider::calcR1x(float vOut) {
  constrain(vOut, 0, _avRef); //Ensure that the passed voltage is within hardware bounds.
  float r1 = doCalcR1x(vOut);
  return r1;
}

float vDivider::doCalcR1x(uint16_t averageADC) {
  float r1 = 0.0;
  if (_balanceResistor > 0.0) {
    float rRatio = calcRRatio(averageADC);
    r1 = _balanceResistor * rRatio;
  }
  return r1;
}

float vDivider::doCalcR1x(float vOut) {
  float r1 = 0.0;
  if (_balanceResistor > 0.0) {
    float rRatio = calcRRatio(vOut);
    r1 = _balanceResistor * rRatio;
  }
  return r1;
}

//Calculate the R2 value assuming that the balance resistor is R1.
float vDivider::calcR2x() {
  //float vOut = calcVOut();
  uint16_t averageADC = readADC();
  //float r2 = doCalcR2x(vOut);
  float r2 = doCalcR1x(averageADC);
  return r2;
}

//Calculate the R2 value using the given ADC reading, and assuming that the balance resistor is R1.
float vDivider::calcR2x(uint16_t averageADC) {
  constrain(averageADC, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float r2 = doCalcR2x(averageADC);
  return r2;
}

//Calculate the R2 value using the given ADC reading, and assuming that the balance resistor is R1.
float vDivider::calcR2x(int averageADC) {
  float r2 = calcR2x((uint16_t)averageADC); //Cast the int to a uint16_t.
  return r2;
}

//Calculate the R2 value using the given divider voltage, and assuming that the balance resistor is R1.
float vDivider::calcR2x(float vOut) {
  constrain(vOut, 0, _avRef); //Ensure that the passed voltage is within hardware bounds.
  float r2 = doCalcR2x(vOut);
  return r2;
}

float vDivider::doCalcR2x(uint16_t averageADC) {
  float r2 = 0.0;
  if (_balanceResistor > 0.0) {
    float rRatio = calcRRatio(averageADC);
    r2 = _balanceResistor / rRatio;
  }
  return r2;
}

float vDivider::doCalcR2x(float vOut) {
  float r2 = 0.0;
  if (_balanceResistor > 0.0) {
    float rRatio = calcRRatio(vOut);
    r2 = _balanceResistor / rRatio;
  }
  return r2;
}

//Calculate the RRatio using the given ADC reading.
float vDivider::calcRRatio(uint16_t averageADC) {
  float rRatio;
  rRatio = ((float)(_adcMax + 1) / (float)averageADC) - 1.0;
  return rRatio;
}

//Calculate the RRatio using the given divider voltage.
float vDivider::calcRRatio(float vOut) {
  float rRatio;
  rRatio = (_avRef / vOut) - 1.0;
  return rRatio;
}

//EOF
