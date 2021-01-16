/*
Arduino library for analogue temperature sensors.

Supported sensors: TMP36, KY013 & MF52D
Other thermistors can also be used if the correct parameters are set.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk

https://arduino.github.io/arduino-cli/library-specification/
https://roboticsbackend.com/arduino-create-library/
https://roboticsbackend.com/arduino-object-oriented-programming-oop/

https://github.com/panStamp/thermistor
https://github.com/Isaac100/TMP36/blob/master/src/TMP36.cpp
https://forum.arduino.cc/index.php?topic=209133.0
https://forum.arduino.cc/index.php?topic=614371.0
https://github.com/themactep/KY013
https://arduinomodules.info/ky-013-analog-temperature-sensor-module/
https://autobotic.com.my/analog-temperature-sensor-module

https://en.wikipedia.org/wiki/Thermistor
https://en.wikipedia.org/wiki/Steinhart-Hart_equation
https://www.ametherm.com/thermistor/ntc-thermistors-steinhart-and-hart-equation
https://www.qtisensing.com/wp-content/uploads/Beta-vs-Steinhart-Hart.pdf
https://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
*/

#include "AlogTSensors.h"

/*!
 *  @brief  Instantiates a new TMP36 class.
 *  @param  pin
 *          IO pin number that the sensor is connected to.
 */

TMP36::TMP36(uint8_t sPin)
      : vDivider(sPin, 0.0) {
  _offset = _TMP36OFFSET;
  _multiplier = _TMP36MULTIPLIER;
}

void TMP36::setParms(float offset, float multiplier) {
  _offset = offset;
  _multiplier = multiplier;
}

float TMP36::readTemperatureC() {
  float vOut = calcVOut();
  float temperatureC = ((vOut - _offset) * _multiplier);
  return temperatureC;
}

float TMP36::readTemperatureC(uint16_t averageADC) {
  float vOut = calcVOut(averageADC);
  float temperatureC = ((vOut - _offset) * _multiplier);
  return temperatureC;
}

float TMP36::readTemperatureC(float vOut) {
  constrain(vOut, 0, _adcMax);  //Ensure that the passed ADC value is within hardware bounds.
  float temperatureC = ((vOut - _offset) * _multiplier);
  return temperatureC;
}

float TMP36::readTemperatureK() {
  float temperatureC = readTemperatureC();
  return convertCelsiusToKelvin(temperatureC);
}

float TMP36::readTemperatureK(uint16_t averageADC) {
  float temperatureC = readTemperatureC(averageADC);
  return convertCelsiusToKelvin(temperatureC);
}

float TMP36::readTemperatureK(float vOut) {
  float temperatureC = readTemperatureC(vOut);
  return convertCelsiusToKelvin(temperatureC);
}

float TMP36::readTemperatureF() {
  float temperatureC = readTemperatureC();
  return convertCelsiusToFahrenheit(temperatureC);
}

float TMP36::readTemperatureF(uint16_t averageADC) {
  float temperatureC = readTemperatureC(averageADC);
  return convertCelsiusToFahrenheit(temperatureC);
}

float TMP36::readTemperatureF(float vOut) {
  float temperatureC = readTemperatureC(vOut);
  return convertCelsiusToFahrenheit(temperatureC);
}

float TMP36::convertCelsiusToKelvin(float temperatureC) {
  return (temperatureC + 273.15);
}

float TMP36::convertCelsiusToFahrenheit(float temperatureC) {
  return ((temperatureC * 9.0) / 5.0) + 32.0;
}

/*!
 *  @brief  Instantiates a new Thermistor class.
 *  @param  pin
 *          IO pin number that the sensor is connected to.
 *  @param  balanceresistor
 *          The fixed, known, resistor in the voltage divider.
 *  @param  isR1
 *          True/False if the balance resistor is R1.
 */

Thermistor::Thermistor(uint8_t sPin, float balanceResistor, bool isR1, bool useCBeta)
      : vDivider(sPin, balanceResistor, isR1) {
  _useCBeta = useCBeta;
  setTType();
}

/*!
 *  @brief  Set the thermistor type and associated SHH values.
 *  @param  tType
 *          The thermistor type.
 */

void Thermistor::setTType(uint16_t tType) {
  _tType = tType;
  switch(_tType) {
    case ATS_KY013:
      setC123(_KY013COEFFICIENT1, _KY013COEFFICIENT2, _KY013COEFFICIENT3);
      break;
    case ATS_MF52D:
      setCBeta(_MF52DCBETA, _MF52DNOMRST, _MF52DNOMTEMP);
      break;
    default:
      if (_useCBeta) {
        _cBeta = _DEFCBETA;
        _nomRst = _DEFNOMRST;
        _nomTemp = _DEFNOMTEMP;
      }
      else {
        _coefficient1 = _DEFCOEFFICIENT1;
        _coefficient2 = _DEFCOEFFICIENT2;
        _coefficient3 = _DEFCOEFFICIENT3;
    }
  }
}

/*!
 *  @brief  Set the thermistor beta values.
 *  @param  cBeta
 *          The thermistor beta parameter value.
 *  @param  nomRst
 *          The thermistor nominal resistance.
 *  @param  nomTemp
 *          The thermistor nominal temperature.
 */

void Thermistor::setCBeta(uint16_t cBeta, float nomRst, float nomTemp) {
  _useCBeta = true;
  _cBeta = cBeta;
  _nomRst = nomRst;
  _nomTemp = nomTemp;
}

/*!
 *  @brief  Set the thermistor c123 values.
 *  @param  coefficient1
 *          The thermistor coefficient 1 parameter value.
 *  @param  coefficient2
 *          The thermistor coefficient 2 parameter value.
 *  @param  coefficient3
 *          The thermistor coefficient 3 parameter value.
 */

void Thermistor::setC123(float coefficient1, float coefficient2, float coefficient3) {
  _useCBeta = false;
  _coefficient1 = coefficient1;
  _coefficient2 = coefficient2;
  _coefficient3 = coefficient3;
}

float Thermistor::readTemperatureK() {
  float tRst;
  if (_isR1) {
    tRst = calcR2();
  }
  else {
    tRst = calcR1();
  }
  float temperatureK = readTemperatureK(tRst);
  return temperatureK;
}

float Thermistor::readTemperatureK(float tRst) {
  float oneOverTK;
  if (_useCBeta) {
    //Calculate (1/T) using the Beta Coefficient formula.
    oneOverTK = (1.0 / (_nomTemp + 273.15)) + ((1.0 / _cBeta) * log((tRst / _nomRst)));
  }
  else {
    //Calculate (1/T) using the Steinhart-Hart (C123) formula.
    float lntRst = log(tRst);
    oneOverTK = _coefficient1 + (_coefficient2 * lntRst) + (_coefficient3 * pow(lntRst, 3));
  }
  return (1.0 / oneOverTK);
}

float Thermistor::readTemperatureC() {
  float temperature = readTemperatureK();
  return convertKelvinToCelsius(temperature);
}

float Thermistor::readTemperatureC(float tRst) {
  float temperatureK = readTemperatureK(tRst);
  return convertKelvinToCelsius(temperatureK);
}

float Thermistor::readTemperatureF() {
  float temperatureC = readTemperatureC();
  return convertCelsiusToFahrenheit(temperatureC);
}

float Thermistor::readTemperatureF(float tRst) {
  float temperatureC = readTemperatureC(tRst);
  return convertCelsiusToFahrenheit(temperatureC);
}

float Thermistor::convertKelvinToCelsius(float temperatureK) {
  return temperatureK - 273.15;
}

float Thermistor::convertCelsiusToFahrenheit(float temperatureC) {
  return ((temperatureC * 9.0) / 5.0) + 32.0;
}

/*!
 *  @brief  Instantiates a new KY013 class.
 *  @param  pin
 *          IO pin number that the sensor is connected to.
 *  @param  balanceresistor
 *          The fixed, known, resistor in the voltage divider.
 *  @param  isR1
 *          True/False if the balance resistor is R1.
 */

KY013::KY013(uint8_t sPin, float balanceResistor, bool isR1)
      : Thermistor(sPin, balanceResistor, isR1) {
  setC123(_KY013COEFFICIENT1, _KY013COEFFICIENT2, _KY013COEFFICIENT3);
}

/*!
 *  @brief  Instantiates a new MF52D class.
 *  @param  pin
 *          IO pin number that the sensor is connected to.
 *  @param  balanceresistor
 *          The fixed, known, resistor in the voltage divider.
 *  @param  isR1
 *          True/False if the balance resistor is R1.
 */

MF52D::MF52D(uint8_t sPin, float balanceResistor, bool isR1)
      : Thermistor(sPin, balanceResistor, isR1) {
  setCBeta(_MF52DCBETA, _MF52DNOMRST, _MF52DNOMTEMP);
}

//EOF

