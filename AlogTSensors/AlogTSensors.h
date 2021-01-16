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

#ifndef ALOGTSENSORS_H
  #define ALOGTSENSORS_H

  #include <Arduino.h>
  #include <VDivider.h>

  //Analog Temperature Sensor type numbers.
  #define ATS_DEFLT 0
  #define ATS_TMP36 36                      //Defined for consistancy, but not used anywhere yet.
  #define ATS_KY013 13
  #define ATS_MF52D 52

  #define _TMP36OFFSET 0.5
  #define _TMP36MULTIPLIER 100.0

  //Default NTC Thermistor calculation values.
  #define _DEFCBETA 3435                    //Beta coefficient.
  #define _DEFNOMRST 10000.0                //Nominal resistance.
  #define _DEFNOMTEMP 25.0                  //Nominal temperature.
  #define _DEFCOEFFICIENT1 0.001129148      //1.129148e-03. Common SHH coefficient 1 found on the Internet.
  #define _DEFCOEFFICIENT2 0.000234125      //2.341250e-04. Common SHH coefficient 2 found on the Internet.
  #define _DEFCOEFFICIENT3 0.0000000876741  //8.767410e-08. Common SHH coefficient 3 found on the Internet.

  //NTC Thermistor Temperature Sensor AZ Deliveries 10K@T25 KY013 module (from ebook datasheet).
  //#define _KY013COEFFICIENT1 0.001009249522     //1.009249522e-03
  //#define _KY013COEFFICIENT2 0.0002378405444    //2.378405444e-04
  //#define _KY013COEFFICIENT3 0.0000002019202697 //2.019202697e-07

  //NTC Thermistor Temperature Sensor AZ Deliveries 100K@T25 KY013 module (calculated with NTCcalculator).
  #define _KY013COEFFICIENT1 0.0005182977433      //5.182977433e-04
  #define _KY013COEFFICIENT2 0.0002252079282      //2.252079282e-04
  #define _KY013COEFFICIENT3 0.0000001615362158   //1.615362158e-07

  //NTC Thermistor Temperature Sensor 10K OHM MF52-103 3435 1%
  #define _MF52DCBETA 3435
  #define _MF52DNOMRST 10000.0
  #define _MF52DNOMTEMP 25.0

  //A diode/voltage based analog temperature sensor.
  class TMP36 : public vDivider {
  public:
    TMP36(uint8_t sPin = _ANALOGPIN);
    void setParms(float offset = _TMP36OFFSET, float multiplier = _TMP36MULTIPLIER);    
    float readTemperatureC();
    float readTemperatureC(uint16_t);
    float readTemperatureC(float);
    float readTemperatureK();
    float readTemperatureK(uint16_t);
    float readTemperatureK(float);
    float readTemperatureF();
    float readTemperatureF(uint16_t);
    float readTemperatureF(float);

  private:
    float _offset;
    float _multiplier;
    float convertCelsiusToKelvin(float temperatureC);
    float convertCelsiusToFahrenheit(float temperatureC);
  };

  //A general class for resistance based analog temperature sensors.
  class Thermistor : public vDivider {
  public:
    Thermistor(uint8_t sPin = _ANALOGPIN, float balanceResistor = _BALANCERESISTOR, bool isR1 = true, bool useCBeta = true);
    void setTType(uint16_t tType = ATS_DEFLT);
    void setCBeta(uint16_t cBeta = _DEFCBETA, float nomRst = _DEFNOMRST, float nomTemp = _DEFNOMTEMP);    
    void setC123(float coefficient1 = _DEFCOEFFICIENT1, float coefficient2 = _DEFCOEFFICIENT2, float coefficient3 = _DEFCOEFFICIENT3);    
    float readTemperatureC();
    float readTemperatureC(float);
    float readTemperatureK();
    float readTemperatureK(float);
    float readTemperatureF();
    float readTemperatureF(float);

  private:
    uint16_t _tType;
    bool _useCBeta;
    float _nomRst;
    float _nomTemp;
    float _cBeta;
    float _coefficient1;
    float _coefficient2;
    float _coefficient3;
    float convertKelvinToCelsius(float temperatureK);
    float convertCelsiusToFahrenheit(float temperatureC);
  };

  class KY013 : public Thermistor {
  public:
    KY013(uint8_t sPin = _ANALOGPIN, float balanceResistor = _BALANCERESISTOR, bool isR1 = true);
  };

  class MF52D : public Thermistor {
  public:
    MF52D(uint8_t sPin = _ANALOGPIN, float balanceResistor = _BALANCERESISTOR, bool isR1 = true);
  };
#endif

//EOF

