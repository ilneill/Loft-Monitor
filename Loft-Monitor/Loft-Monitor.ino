/*
Loft Environment Monitor Sensor Data Collector - Sensor Trial and Study.

(c) 2020-2021 Ian Neill, arduino@binaria.co.uk
Licence: GPLv3

  Temperature/Humidity Sensors
  - BME280 sensor : Digital, best specifications, prone to self-heating.
  - DHT11 sensor  : Digital, curiosity, poor specifications.
  - DHT22 sensor  : Digital, improved specifications over DHT11, but slower sampling.
  Temperature Sensors
  - DS18B20 sensor: Digital, good specifications, and on a long lead.
  - KY013 sensor  : Analog, good specifications, a non-linear response, but difficult to determine coefficients.
  - TMP36 sensor  : Analog, very good specifications, a linear response, and very easy to use.
  - MF52D sensor  : Analog, good specifications, a non-linear response, and easy to use.
  Light Sensors
  - LDR sensor    : Analog, unknown specifications, unknown response, but easy to work with.

20210110  01.00 First shared release.
20210115  01.01 First public release.
                Bug fixes and further code optimisations.
*/

//https://roboticsbackend.com/arduino-object-oriented-programming-oop/

//https://lastminuteengineers.com/bme280-arduino-tutorial/
//https://simple-circuit.com/weather-station-arduino-bme280-ssd1306/
//https://www.lpomykal.cz/bme280-complex-measurement/
//https://github.com/letscontrolit/ESPEasy/issues/164
//https://www.lpomykal.cz/bme280-temperature-measurement/
//https://www.instructables.com/id/Ultimate-Guide-to-Adruino-Serial-Plotter/
//https://www.instructables.com/id/How-to-use-DHT-22-sensor-Arduino-Tutorial/
//https://www.makerguides.com/dht11-dht22-arduino-tutorial/
//https://toptechboy.com/arduino-tutorial-50-how-to-connect-and-use-the-dht11-temperature-and-humidity-sensor/
//https://www.electroniclinic.com/temperature-controlled-fan-or-room-cooler-using-arduino/
//https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf
//https://randomnerdtutorials.com/dht11-vs-dht22-vs-lm35-vs-ds18b20-vs-bme280-vs-bmp180/
//https://randomnerdtutorials.com/9-arduino-compatible-temperature-sensors-for-your-electronics-projects/
//https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
//https://www.makerguides.com/ds18b20-arduino-tutorial/
//https://www.pjrc.com/teensy/td_libs_OneWire.html
//https://github.com/milesburton/Arduino-Temperature-Control-Library
//https://microcontrollerslab.com/analog-voltage-reading-using-arduino-uno-r3/
//https://github.com/themactep/KY013
//https://arduinomodules.info/ky-013-analog-temperature-sensor-module/
//https://autobotic.com.my/analog-temperature-sensor-module
//https://en.wikipedia.org/wiki/Voltage_divider
//https://en.wikipedia.org/wiki/Thermistor
//https://en.wikipedia.org/wiki/Steinhart-Hart_equation
//https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
//https://bc-robotics.com/tutorials/using-a-tmp36-temperature-sensor-with-arduino/
//https://github.com/Isaac100/TMP36
//https://github.com/amperka/TroykaThermometer
//https://forum.arduino.cc/index.php?topic=614371.0
//http://www.eaa.net.au/PDF/Hitech/MF52type.pdf
//http://files.rct.ru/pdf/thermistor/ntc_mf52.pdf

//Read data from the BME280, DHT11, DHT22, DS18B20, KY013, TMP36 & MF52D temperature sensors, and send it to the serial console,
//  and then update some temperature band LEDs.
//The light level is also recorded, using an LDR sensor, and sent to the serial console.
//
//Band 7: Red + White(F) = more than 55 deg C
//Band 6: Red + White = 45 --> 55 deg C
//Band 5: Red = 35 --> 45 deg C
//Band 4: Yellow = 25 --> 35 deg C
//Band 3: Green = 5 --> 25 deg C
//Band 2: Blue = -15 --> 5 deg C
//Band 1: Blue + White = -25 --> -15 deg C
//Band 0: Blue + White(F) = less than -25 deg C

//ToDo:
//1. BME280 read speed reduction - completed.
//2. Band transition hysteresis - completed.
//3. Output data suitable for graphing - completed.
//4. Add a DHT11 sensor - completed.
//5. LCD display support - dropped, not enough I/O pins anyway.
//6. Bluetooth support - dropped, going to serial log instead.
//7. Add a DHT22 sensor - completed.
//8. Add a definable data delimiter for ploting output - completed.
//9. Add the option to use the average temperature from all enabled sensors - completed.
//A. Add a fourth data sensor - DS18B20 - completed.
//B. Add Heat Index calculation and display - dropped, due to time and code size concerns.
//C. Improve average temperature options - completed.
//D. Add a fifth data sensor - KY013 - completed (coefficient trouble resolved).
//E. Correct the KYO13 library voltage divider calculations - completed.
//F. Get a variable from a class, specifically DHT sensor type - completed, then dropped as it breaks 3rd party library updates.
//G. Add a sixth data sensor - TMP36 - completed.
//H. Add a discrete MF52D-103f-3435 NTC thermistor - completed.
//I. Add an LDR light sensor - completed.
//J. Follow up KY013 coefficients with AZ-Deliveries - dropped, useless response (used NTCcalculator instead).
//K. Think about plot data timestamping - completed: the receiver must do this, as the arduino cannot - using CoolTerm on Windows.
//L. Review code looking for any more code optimisations - completed (several times, and always on-going).
//M. Compile code with all->no sensors enabled, for SDEBUG and PLOTDATA options - completed.
//N. Add an average humidity option?
//O. Add support for multiple DS18B20 sensors on the OneWire bus.

//Code Optimisation
//https://learn.adafruit.com/memories-of-an-arduino/optimizing-program-memory
//https://subethasoftware.com/2014/02/20/arduino-memory-and-optimizations/
#define USE_FLASH                 //Enable this to allow the F() macro to keep constant strings in flash, and out of RAM.
#ifdef USE_FLASH
  #define FLASHSTR(x) F(x)        //Substitute with the F() macro.
#else
  #define FLASHSTR(x) (x)         //Substitute with just the original string.
#endif

#include <Wire.h>                 //Required for I2C bus access.
#include <OneWire.h>              //Required for One Wire bus access.
#include <Adafruit_Sensor.h>      //Common library for BME280 and DHT11/22 sensors.
#include <Adafruit_BME280.h>      //BME280 sensor library.
#include <DHT.h>                  //DHT11/22 sensor library.
#include <DallasTemperature.h>    //DS18B20 sensor library.
#include <VDivider.h>             //My own analog voltage divider library, used for the LDR and the analog temperature sensors.
#include <AlogTSensors.h>         //My own analog sensor library for KY013, MF52D and TMP36 temperature sensors. Inherits from VDivider.h.

#define SCRIPT_NAME FLASHSTR("Binaria Loft Environment Monitor v1.01")

#define AVREF 4.83                //Actual measured 5V reference voltage. Important for the TMP36 sensor accuracy.
#define ADCMAXVALUE 1023          //Maximum ADC value for a 10bit ADC - used in some of the pseudo sensor functions.

//BME280 sensor defines.
#define BME280_ENABLED
#define BME280_I2C_ADDRESS 0x76
//#define GETPRESSURE
//#define GETALTITUDE
#define SEALEVELPRESSURE_HPA 1013.25

//DHT11 sensor defines.
#define DHT11_ENABLED
#define DHT_TYPE11 DHT11          //Blue module, DHT11 defined as 11 in <DHT.h>.

//DHT22 sensor defines.
#define DHT22_ENABLED
#define DHT_TYPE22 DHT22          //White module, DHT22 defined as 22 in <DHT.h>.

//DS18B20 sensor defines.
#define DS18B20_ENABLED

//KY013 sensor defines - NTC Thermistor Temperature Sensor AZ Deliveries KY013, 100K@T25 nominal resistance.
#define KY013_ENABLED
#define KY013_SAMPLES 32
#define KY013R1 110000            //110K balance resistor (100K external + 10K on board).
#define C1_KY 0.0005182977433     //Steinhart-Hart coefficient 1, 5.182977433e-04.
#define C2_KY 0.0002252079282     //Steinhart-Hart coefficient 2, 2.252079282e-04.
#define C3_KY 0.0000001615362158  //Steinhart-Hart coefficient 3, 1.615362158e-07.

//TMP36 sensor defines.
#define TMP36_ENABLED
#define TMP36_SAMPLES 32
#define TMP36_SDELAY 1
#define TMP36_ADCRDYDLY 20

//MF52D sensor defines - NTC Thermistor Temperature Sensor 10K OHM MF52-103 3435 1%.
#define MF52D_ENABLED
#define MF52D_SAMPLES 32
#define MF52DR1 10000             //10K balance resistor.
#define CBETA_MF 3435             //Beta coefficient.
#define NOMRST_MF 10000           //Nominal resistance.
#define NOMTEMP_MF 25.0           //Nominal temperature.

//LDR sensor defines.
#define LDR_ENABLED
#define LDR_SAMPLES 32
#define LDRR2 100000              //100K balance resistor.

//Check that some sensors are enabled.
#if not (defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED) || defined(DS18B20_ENABLED) || defined(KY013_ENABLED) || defined(TMP36_ENABLED) || defined(MF52D_ENABLED) || defined(LDR_ENABLED))
  #error "Sketch compilation STOPPED - No sensors are enabled!"
#endif
//Check if no temperature sensors are enabled.
#if not (defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED) || defined(DS18B20_ENABLED) || defined(KY013_ENABLED) || defined(TMP36_ENABLED) || defined(MF52D_ENABLED))
  #warning "Sketch compilation PROBLEM - No temperature sensors are enabled!"
#endif

//Arduino analog I/O pin defines.
#define KY013_PIN A0              //Also known as IO pin 14.
#define TMP36_PIN A1              //Also known as IO pin 15.
#define MF52D_PIN A2              //Also known as IO pin 16.
#define LDR_PIN A3                //Also known as IO pin 17.
//define I2C_SDA_PIN A4           //Also known as IO pin 18. Used by the I2C bus.
//define I2C_SCL_PIN A5           //Also known as IO pin 19. Used by the I2C bus.
#define SPARE_PIN A6              //Also known as IO pin 20.
#define POT_PIN A7                //Also known as IO pin 21. Potentiometer input used for code testing purposes only.

//Arduino digital I/O pin defines.
#define HB_LED 13
#define DS18B20_PIN 2
#define DHT11_PIN 3
#define DHT22_PIN 4
#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BLUE_LED 8
#define WHITE_LED 9
#define LED_TEST_DELAY 500

//Debugging & testing define.
//#define SDEBUG                  //Enable code testing and output simulation with pseudo sensor data from a potentiometer +/- randomness.

//Data capture defines.
#define PLOTDATA                  //Enable serial port output designed to be captured to a file for later analysis, trending, and graphing.
//#define DATADELIMITER FLASHSTR(" ")
//#define DATADELIMITER FLASHSTR("\t")
#define DATADELIMITER FLASHSTR(",")

//Temperature band defines.
#define USEAVERAGETEMP            //Calculate an average sensor temperature and use it to update the temperature band LEDs.

//All these thresholds are in deg C.
#define UHOT_BAND 55.0            //Band 7 - Same LEDs as VHOT, but the white LED is blinked.
#define VHOT_BAND 45.0            //Band 6.
#define RED_BAND 35.0             //Band 5.
#define YELLOW_BAND 25.0          //Band 4.
#define GREEN_BAND 5.0            //Band 3.
#define BLUE_BAND -15.0           //Band 2.
#define VCOLD_BAND -25.0          //Band 1.
#define UCOLD_BAND -274.0         //Band 0 - Same LEDs as VCOLD, but the white LED is blinked.
#define BAND_HYSTERESIS 0.5       //Stop jitter if the temperature is around a band boundry.

//Code loop variables and defines.
bool hbStatus = LOW;                          //Heartbeat status.
bool wlStatus = LOW;                          //White LED status.
byte temperatureBand = 3;                     //Initial temperature band value (green band).
#define loopDelayTime 500                     //Loop delay in ms.
#define sampleEvery 30                        //Read the sensors every X loop delays. Ensure sensor readings are not faster than once every 5 secs.
byte sampleCountDown = sampleEvery / 3;       //Sample delay count down start value.
#ifdef USEAVERAGETEMP
  byte numTSensors = 0;                       //Average temperature divider.
#endif

//Sensor object initialisations.
#ifndef SDEBUG
  #ifdef BME280_ENABLED
    Adafruit_BME280 myBME280;                 //Initialise the BME280 sensor using I2C interface, SCL = A5, SDA = A4.
  #endif
  #ifdef DHT11_ENABLED
    DHT myDHT11(DHT11_PIN, DHT_TYPE11);       //Initialize the DHT11 sensor for normal 16mhz Arduino (default delay = 6).
  #endif
  #ifdef DHT22_ENABLED
    DHT myDHT22(DHT22_PIN, DHT_TYPE22);       //Initialize the DHT22 sensor for normal 16mhz Arduino (default delay = 6).
  #endif
  #ifdef DS18B20_ENABLED
    OneWire oneWireBus(DS18B20_PIN);          //Setup a oneWire instance to communicate with the DS18B20 sensor.
    DallasTemperature myDS18B20(&oneWireBus); //Initialise the DS18B20 sensor.
  #endif
  #ifdef KY013_ENABLED
    KY013 myKY013(KY013_PIN, KY013R1);        //Initialise the KY013 sensor. The balance resistor is R1.
  #endif
  #ifdef TMP36_ENABLED
    TMP36 myTMP36(TMP36_PIN);                 //Initialise the TMP36 sensor. This sensor does not use a balance resistor.
  #endif
  #ifdef MF52D_ENABLED
    MF52D myMF52D(MF52D_PIN, MF52DR1);        //Initialise the MF52D sensor. The balance resistor is R1.
  #endif
  #ifdef LDR_ENABLED
    vDivider myLDR(LDR_PIN, LDRR2, false);    //Initialise the LDR sensor. The balance resistor is R2 (not R1).
  #endif
#endif

void setup() {
  #ifndef SDEBUG
    #ifdef BME280_ENABLED
      bool bme280_status;
    #endif
  #endif
  //Set up the heartbeat LED.
  pinMode(HB_LED, OUTPUT);
  digitalWrite(HB_LED, hbStatus);
  //Set up LED pins.
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  //Set up the analog pins.
  #ifdef SDEBUG
    pinMode(POT_PIN, INPUT);
    randomSeed(analogRead(POT_PIN));
  #endif
  //Start the serial console.
  Serial.begin(9600);
  while(!Serial); //Wait for the serial I/O to start.
  delay(850);
  #ifndef PLOTDATA
    Serial.println();
    Serial.println(SCRIPT_NAME);
    Serial.println();
    delay(150);
  #endif
  #ifndef SDEBUG
    //Start the BME280 sensor.
    #ifdef BME280_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("BME280 starting:"));
      #endif
      bme280_status = myBME280.begin(BME280_I2C_ADDRESS);
      #ifndef PLOTDATA
        Serial.print(FLASHSTR(" -> Status = "));
        Serial.println(bme280_status);
      #endif
      if (!bme280_status) {
        #ifndef PLOTDATA
          Serial.println(FLASHSTR(" -> BME280 sensor not found - check the wiring or the I2C address!"));
          Serial.println(FLASHSTR(" -> Code HALTed!"));
        #endif
        while(true) {
          //ALERT! Rapidly flash the Heartbeat LED.
          hbStatus = !hbStatus;
          digitalWrite(HB_LED, hbStatus);
          delay(loopDelayTime / 5);
        }
      }
      //Set up the BME280 for forced mode reading.
      myBME280.setSampling(Adafruit_BME280::MODE_FORCED,  //Force reading after delayTime.
                           Adafruit_BME280::SAMPLING_X1,  //Temperature sampling set to 1.
                           Adafruit_BME280::SAMPLING_X1,  //Pressure sampling set to 1.
                           Adafruit_BME280::SAMPLING_X1,  //Humidity sampling set to 1.
                           Adafruit_BME280::FILTER_OFF);  //Filter off - immediate 100% step response.
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> BME280 forced mode enabled."));
        //Serial.println();
      #endif
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the DHT11 sensors.
    #ifdef DHT11_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("DHT11 starting:"));
      #endif
      myDHT11.begin();
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the DHT22 sensor.
    #ifdef DHT22_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("DHT22 starting:"));
      #endif
      myDHT22.begin();
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the DS18B20 sensor.
    #ifdef DS18B20_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("DS18B20 starting:"));
      #endif
      myDS18B20.begin();
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the KY013 sensor.
    #ifdef KY013_ENABLED
      //Start the KY013 sensor.
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("KY013 starting:"));
      #endif
      myKY013.setConsts(KY013_SAMPLES);
      myKY013.setC123(C1_KY, C2_KY, C3_KY);
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the TMP36 sensor.
    #ifdef TMP36_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("TMP36 starting:"));
      #endif
      myTMP36.setConsts(TMP36_SAMPLES, TMP36_SDELAY, TMP36_ADCRDYDLY, AVREF);
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the MF52D sensor.
    #ifdef MF52D_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("MF52D starting:"));
      #endif
      myMF52D.setConsts(MF52D_SAMPLES);
      myMF52D.setCBeta(CBETA_MF, NOMRST_MF, NOMTEMP_MF);
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
      #ifdef USEAVERAGETEMP
        numTSensors++;
      #endif
    #endif
    //Start the LDR sensor.
    #ifdef LDR_ENABLED
      #ifndef PLOTDATA
        Serial.println(FLASHSTR("LDR starting:"));
      #endif
      myLDR.setConsts(LDR_SAMPLES);
      #ifndef PLOTDATA
        Serial.println(FLASHSTR(" -> OK"));
      #endif
    #endif
    //Separator.
    #ifndef PLOTDATA
      Serial.println();
    #endif
  #else
    #ifdef USEAVERAGETEMP
      #ifdef BME280_ENABLED
        numTSensors++;
      #endif
      #ifdef DHT11_ENABLED
        numTSensors++;
      #endif
      #ifdef DHT22_ENABLED
        numTSensors++;
      #endif
      #ifdef DS18B20_ENABLED
        numTSensors++;
      #endif
      #ifdef KY013_ENABLED
        numTSensors++;
      #endif
      #ifdef TMP36_ENABLED
        numTSensors++;
      #endif
      #ifdef MF52D_ENABLED
        numTSensors++;
      #endif
    #endif
  #endif
  //Test the LEDs.
  #ifndef PLOTDATA
    Serial.println(FLASHSTR("LED Tests:"));
    Serial.print(FLASHSTR(" -> ON : R"));
  #endif
    digitalWrite(RED_LED, HIGH);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("Y"));
  #endif
    digitalWrite(YELLOW_LED, HIGH);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("G"));
  #endif
    digitalWrite(GREEN_LED, HIGH);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("B"));
  #endif
    digitalWrite(BLUE_LED, HIGH);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.println(FLASHSTR("W"));
  #endif
    digitalWrite(WHITE_LED, HIGH);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR(" -> OFF: R"));
  #endif
    digitalWrite(RED_LED, LOW);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("Y"));
  #endif
    digitalWrite(YELLOW_LED, LOW);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("G"));
  #endif
    digitalWrite(GREEN_LED, LOW);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("B"));
  #endif
    digitalWrite(BLUE_LED, LOW);
    delay(LED_TEST_DELAY);
  #ifndef PLOTDATA
    Serial.println(FLASHSTR("W"));
  #endif
    digitalWrite(WHITE_LED, LOW);
    delay(LED_TEST_DELAY);
  //Send the plot column names.
  #ifdef PLOTDATA
    #ifdef BME280_ENABLED
      Serial.print(FLASHSTR("Temperature(BME280)"));
      Serial.print(DATADELIMITER);
      Serial.print(FLASHSTR("Humidity(BME280)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef DHT11_ENABLED
      Serial.print(FLASHSTR("Temperature(DHT11)"));
      Serial.print(DATADELIMITER);
      Serial.print(FLASHSTR("Humidity(DHT11)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef DHT22_ENABLED
      Serial.print(FLASHSTR("Temperature(DHT22)"));
      Serial.print(DATADELIMITER);
      Serial.print(FLASHSTR("Humidity(DHT22)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef DS18B20_ENABLED
      Serial.print(FLASHSTR("Temperature(DS18B20)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef KY013_ENABLED
      Serial.print(FLASHSTR("Temperature(KY013)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef TMP36_ENABLED
      Serial.print(FLASHSTR("Temperature(TMP36)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef MF52D_ENABLED
      Serial.print(FLASHSTR("Temperature(MF52D)"));
      Serial.print(DATADELIMITER);
    #endif
    #ifdef LDR_ENABLED
      Serial.print(FLASHSTR("Light-Level(LDR)"));
      Serial.print(DATADELIMITER);
    #endif
    Serial.println(FLASHSTR("Temperature-Band"));
  #else
    Serial.println(); //Data block separator.
  #endif
}

void loop() {
  #ifdef BME280_ENABLED
    float temperature_BME280;
  #endif
  #ifdef DHT11_ENABLED
    float temperature_DHT11;
  #endif
  #ifdef DHT22_ENABLED
    float temperature_DHT22;
  #endif
  #ifdef DS18B20_ENABLED
    float temperature_DS18B20;
  #endif
  #ifdef KY013_ENABLED
    float temperature_KY013;
  #endif
  #ifdef TMP36_ENABLED
    float temperature_TMP36;
  #endif
  #ifdef MF52D_ENABLED
    float temperature_MF52D;
  #endif
  #ifdef USEAVERAGETEMP
    float totalTemperature = 0.0;
  #endif
  //Toggle the heartbeat LED.
  hbStatus = !hbStatus;
  digitalWrite(HB_LED, hbStatus);
  //Toggle the white LED if the temperature band is 0 (ucold) or 7 (uhot).
  if (temperatureBand == 0 or temperatureBand == 7) {
    wlStatus = !wlStatus;
    digitalWrite(WHITE_LED, wlStatus);
  }
  //Get the BME280, DHT11, DHT22, DS18B20, KY013, TMP36, MF52D and LDR sensor data.
  if (--sampleCountDown <= 0) {
    #ifndef PLOTDATA
      Serial.println(); //Data block separator.
    #endif
    sampleCountDown = sampleEvery;
    #ifdef SDEBUG
      #ifdef BME280_ENABLED
        char sensorNameBME280[] = "BME280";
        temperature_BME280 = getPseudoTemp(sensorNameBME280);
        getPseudoHumidity(sensorNameBME280);
      #endif
      #ifdef DHT11_ENABLED
        char sensorNameDHT11[] = "DHT11";
        temperature_DHT11 = getPseudoTemp(sensorNameDHT11);
        getPseudoHumidity(sensorNameDHT11);
      #endif
      #ifdef DHT22_ENABLED
        char sensorNameDHT22[] = "DHT22";
        temperature_DHT22 = getPseudoTemp(sensorNameDHT22);
        getPseudoHumidity(sensorNameDHT22);
      #endif
      #ifdef DS18B20_ENABLED
        char sensorNameDS18B20[] = "DS18B20";
        temperature_DS18B20 = getPseudoTemp(sensorNameDS18B20);
      #endif
      #ifdef KY013_ENABLED
        char sensorNameKY013[] = "KY013";
        temperature_KY013 = getPseudoTemp(sensorNameKY013);
      #endif
      #ifdef TMP36_ENABLED
        char sensorNameTMP36[] = "TMP36";
        temperature_TMP36 = getPseudoTemp(sensorNameTMP36);
      #endif
      #ifdef MF52D_ENABLED
        char sensorNameMF52D[] = "MF52D";
        temperature_MF52D = getPseudoTemp(sensorNameMF52D);
      #endif
      #ifdef LDR_ENABLED
        getPseudoLight();
      #endif
    #else
      //Get the BME280 sensor data.
      #ifdef BME280_ENABLED
        temperature_BME280 = getBME280data();
      #endif
      //Get DHT11 sensor data.
      #ifdef DHT11_ENABLED
        temperature_DHT11 = getDHT11data();
      #endif
      //Get DHT22 sensor data.
      #ifdef DHT22_ENABLED
        temperature_DHT22 = getDHT22data();
      #endif
      //Get DS18B20 sensor data.
      #ifdef DS18B20_ENABLED
        temperature_DS18B20 = getDS18B20data();
      #endif
      //Get KY013 sensor data.
      #ifdef KY013_ENABLED
        temperature_KY013 = getKY013data();
      #endif
      //Get TMP36 sensor data.
      #ifdef TMP36_ENABLED
        temperature_TMP36 = getTMP36data();
      #endif
      //Get MF52D sensor data.
      #ifdef MF52D_ENABLED
        temperature_MF52D = getMF52Ddata();
      #endif
      //Get LDR sensor data.
      #ifdef LDR_ENABLED
        getLDRdata();
      #endif
    #endif    
    //Update the LED indicators, if possible.
    #ifndef USEAVERAGETEMP
      //Update with the first enabled temperature sensor, or not at all if none are enabled. 
      #if defined(BME280_ENABLED)
        updateLEDS(temperature_BME280);
      #elif defined(DHT11_ENABLED)
        updateLEDS(temperature_DHT11);
      #elif defined(DHT22_ENABLED)
        updateLEDS(temperature_DHT22);
      #elif defined(DS18B20_ENABLED)
        updateLEDS(temperature_DS18B20);
      #elif defined(KY013_ENABLED)
        updateLEDS(temperature_KY013);
      #elif defined(TMP36_ENABLED)
        updateLEDS(temperature_TMP36);
      #elif defined(MF52D_ENABLED)
        updateLEDS(temperature_MF52D);
      #endif
    #else
      //Update with the average temperature across the enabled temperature sensors.
      #ifdef BME280_ENABLED
        totalTemperature += temperature_BME280;
      #endif
      #ifdef DHT11_ENABLED
        totalTemperature += temperature_DHT11;
      #endif
      #ifdef DHT22_ENABLED
        totalTemperature += temperature_DHT22;
      #endif
      #ifdef DS18B20_ENABLED
        totalTemperature += temperature_DS18B20;
      #endif
      #ifdef KY013_ENABLED
        totalTemperature += temperature_KY013;
      #endif
      #ifdef TMP36_ENABLED
        totalTemperature += temperature_TMP36;
      #endif
      #ifdef MF52D_ENABLED
        totalTemperature += temperature_MF52D;
      #endif
      #if defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED) || defined(DS18B20_ENABLED) || defined(KY013_ENABLED) || defined(TMP36_ENABLED) || defined(MF52D_ENABLED)
        updateLEDS(totalTemperature / numTSensors); //This will only be compiled if there is at least one temperature sensor enabled.
      #endif
    #endif
    Serial.println();               //Blank line or new line after data display.
  }
  else {
    #ifndef PLOTDATA
      Serial.print(FLASHSTR("."));  //Display dots while waiting for the next data read.
    #endif
  }
  delay(loopDelayTime);
}

#ifndef SDEBUG
  #ifdef BME280_ENABLED
    float getBME280data() {
      char sensorName[] = "BME280";
      float temperature;
      float humidity;
      #ifdef GETPRESSURE
        float bmePressure;
      #endif
      #ifdef GETALTITUDE
        float bmeAltitude;
      #endif
      myBME280.takeForcedMeasurement(); //Only needed in forced mode!
      temperature = myBME280.readTemperature();
      humidity = myBME280.readHumidity();
      #ifdef GETPRESSURE
        bmePressure = myBME280.readPressure();
      #endif
      #ifdef GETALTITUDE
        bmeAltitude = myBME280.readAltitude(SEALEVELPRESSURE_HPA);
      #endif
        showTemperature(sensorName, temperature);
        showHumidity(sensorName, humidity);
      #ifndef PLOTDATA
        #ifdef GETPRESSURE
          Serial.print(FLASHSTR("Pressure    (BME280)\t= "));
          Serial.print(bmePressure / 100.0);
          Serial.println(FLASHSTR("hPa"));
        #endif
        #ifdef GETALTITUDE
          Serial.print(FLASHSTR("Altitude    (BME280)\t= "));
          Serial.print(bmeAltitude);
          Serial.println(FLASHSTR("m"));
        #endif
      #endif
      return temperature;
    }
  #endif
  
  #if defined(DHT11_ENABLED)
    float getDHT11data() {
      char sensorName[] = "DHT11";
      float temperature;
      float humidity;
      temperature = myDHT11.readTemperature();
      humidity = myDHT11.readHumidity();
      sendDHT1122data(sensorName, temperature, humidity);
      return temperature;
    }
  #endif
  
  #if defined(DHT22_ENABLED)
    float getDHT22data() {
      char sensorName[] = "DHT22";
      float temperature;
      float humidity;
      temperature = myDHT22.readTemperature();
      humidity = myDHT22.readHumidity();
      sendDHT1122data(sensorName, temperature, humidity);
      return temperature;
    }
  #endif
  
  #if defined(DHT11_ENABLED) || defined(DHT22_ENABLED)
    void sendDHT1122data(char *sensorName, float temperature, float humidity) {
      //Check if any DHT11/DHT22 data reads failed.
      if (isnan(temperature) || isnan(humidity)) {
        temperature = 0.0;
        humidity = 0.0;
        #ifndef PLOTDATA
          Serial.print(FLASHSTR("Failed to read data from the "));
          Serial.print(sensorName);
          Serial.println(FLASHSTR(" sensor!"));
        #else
          spPlotData(temperature);
          spPlotData(humidity);
        #endif
      }
      else {
        showTemperature(sensorName, temperature);
        showHumidity(sensorName, humidity);
      }
    }
  #endif
  
  #ifdef DS18B20_ENABLED
    float getDS18B20data() {
      char sensorName[] = "DS18B20";
      float temperature;
      myDS18B20.requestTemperatures();            //Send the command to get the temperature.
      temperature = myDS18B20.getTempCByIndex(0); //Get the temperature from the first sensor on the bus.
      //Check if the DS18B20 data read failed.
      if (temperature == DEVICE_DISCONNECTED_C) {
        temperature = 0.0;
        #ifndef PLOTDATA
          Serial.println(FLASHSTR("Failed to read data from the DS18B20 sensor!"));
        #else
          spPlotData(temperature);
        #endif
      }
      else {
        showTemperature(sensorName, temperature);
      }
      return temperature;
    }
  #endif
  
  #ifdef KY013_ENABLED
    float getKY013data() {
      char sensorName[] = "KY013";
      float temperature = myKY013.readTemperatureC();
      showTemperature(sensorName, temperature);
      return temperature;
    }
  #endif
  
  #ifdef TMP36_ENABLED
    float getTMP36data() {
      char sensorName[] = "TMP36";
      float temperature = myTMP36.readTemperatureC();
      showTemperature(sensorName, temperature);
      return temperature;
    }
  #endif
  
  #ifdef MF52D_ENABLED
    float getMF52Ddata() {
      char sensorName[] = "MF52D";
      float temperature = myMF52D.readTemperatureC();
      showTemperature(sensorName, temperature);
      return temperature;
    }
  #endif
  
  #ifdef LDR_ENABLED
    void getLDRdata() {
      unsigned int lightLevel;
      lightLevel = myLDR.readADC();
      #ifndef PLOTDATA
        Serial.print(FLASHSTR("Light Level (LDR)\t= "));
        Serial.println(lightLevel);
      #else
        Serial.print(lightLevel);
        Serial.print(DATADELIMITER);    
      #endif
    }
  #endif

#else
  #if defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED) || defined(DS18B20_ENABLED) || defined(KY013_ENABLED) || defined(TMP36_ENABLED) || defined(MF52D_ENABLED)
    float getPseudoTemp(char *sensorName) {
      unsigned int potVal;
      float randDelta;
      float temperature;
      //Read the potentiometer value and convert it to a pseudo temperature.
      potVal = analogRead(POT_PIN);
      randDelta = random(-999, 999) / 1000.0;             //Get some randomness into the mix.
      temperature = ((potVal / 10.0) - 30.0) + randDelta; //Gives fractional results too.
      showTemperature(sensorName, temperature);
      return temperature;
    }
  #endif

  #if defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED)
    void getPseudoHumidity(char *sensorName) {
      unsigned int potVal;
      signed char randDelta;
      float humidity;
      //Read the potentiometer value and convert it to a pseudo humidity.
      potVal = analogRead(POT_PIN);
      randDelta = random(-99, 99);                                          //Get some randomness into the mix.
      humidity = (map(potVal, 0, ADCMAXVALUE, 99, 901) + randDelta) / 10.0; //Gives fractional results too.
      showHumidity(sensorName, humidity);
     }
  #endif

  #ifdef LDR_ENABLED
    void getPseudoLight() {
      unsigned int potVal;
      signed char randDelta;
      unsigned int lightLevel;
      //Read the potentiometer value and convert it to a pseudo light level.
      potVal = analogRead(POT_PIN);
      randDelta = random(-9, 9);                                      //Get some randomness into the mix.
      lightLevel = map(potVal, 0, ADCMAXVALUE, 109, 891) + randDelta; //Restrict the range (100 - 900) much like an LDC does.
      #ifndef PLOTDATA
        Serial.print(FLASHSTR("Light Level (LDR)\t= "));
        Serial.println(lightLevel);
      #else
        Serial.print(lightLevel);
        Serial.print(DATADELIMITER);    
      #endif
    }
  #endif
#endif

#if defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED) || defined(DS18B20_ENABLED) || defined(KY013_ENABLED) || defined(TMP36_ENABLED) || defined(MF52D_ENABLED)
  void showTemperature(char *sensorName, float temperature) {
    #ifndef PLOTDATA
      Serial.print(FLASHSTR("Temperature ("));
      Serial.print(sensorName);
      Serial.print(FLASHSTR(")\t= "));
      Serial.print(temperature);
      Serial.print(FLASHSTR("\xC2\xB0"));
      Serial.println(FLASHSTR("C"));
    #else
      spPlotData(temperature);
    #endif
  }
#endif

#if defined(BME280_ENABLED) || defined(DHT11_ENABLED) || defined(DHT22_ENABLED)
void showHumidity(char *sensorName, float humidity) {
    #ifndef PLOTDATA
      Serial.print(FLASHSTR("Humidity    ("));
      Serial.print(sensorName);
      Serial.print(FLASHSTR(")\t= "));
      Serial.print(humidity);
      Serial.println(FLASHSTR("%"));
    #else
      spPlotData(humidity);
    #endif
  }
#endif

#ifdef PLOTDATA
  void spPlotData(float sensorReading) {
      Serial.print(sensorReading);
      Serial.print(DATADELIMITER);    
  }
#endif

void updateLEDS(float temperature) {
  //Band transition variables.
  float uhot_band;    //Temperature band 7.
  float vhot_band;    //Temperature band 6.
  float red_band;     //Temperature band 5.
  float yellow_band;  //Temperature band 4.
  float green_band;   //Temperature band 3.
  float blue_band;    //Temperature band 2.
  float vcold_band;   //Temperature band 1.
  //float ucold_band;   //Temperature band 0. Variable not actually needed.
  //Hysteresis - adjust the band thresholds dependant upon the current temperature band.
  if (temperatureBand == 7) {       //If we are currently in the uhot band.
    uhot_band = UHOT_BAND - BAND_HYSTERESIS;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 6) {  //If we are currently in the vhot band.
    uhot_band = UHOT_BAND + BAND_HYSTERESIS;
    vhot_band = VHOT_BAND - BAND_HYSTERESIS;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 5) {  //If we are currently in the red band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND + BAND_HYSTERESIS;
    red_band = RED_BAND - BAND_HYSTERESIS;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 4) {  //If we are currently in the yellow band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND + BAND_HYSTERESIS;
    yellow_band = YELLOW_BAND - BAND_HYSTERESIS;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 3) {  //If we are currently in the green band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND + BAND_HYSTERESIS;
    green_band = GREEN_BAND - BAND_HYSTERESIS;
    blue_band = BLUE_BAND;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 2) {  //If we are currently in the blue band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND + BAND_HYSTERESIS;
    blue_band = BLUE_BAND - BAND_HYSTERESIS;
    vcold_band = VCOLD_BAND;
    //ucold_band = UCOLD_BAND;
  }
  else if (temperatureBand == 1) {  //If we are currently in the vcold band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND + BAND_HYSTERESIS;    
    vcold_band = VCOLD_BAND - BAND_HYSTERESIS;
    //ucold_band = UCOLD_BAND;
  }
  else {                            //We must currently be in the ucold band.
    uhot_band = UHOT_BAND;
    vhot_band = VHOT_BAND;
    red_band = RED_BAND;
    yellow_band = YELLOW_BAND;
    green_band = GREEN_BAND;
    blue_band = BLUE_BAND;    
    vcold_band = VCOLD_BAND + BAND_HYSTERESIS;
    //ucold_band = UCOLD_BAND;
  }
//Work out the new temperature band.
  #ifndef PLOTDATA
    Serial.print(FLASHSTR("Alert Temperature\t: "));
    Serial.print(temperature);
    Serial.print(FLASHSTR("\xC2\xB0"));
    Serial.println(FLASHSTR("C"));
    Serial.print(FLASHSTR("!LED Alert Level!\t: "));
  #endif
  if (temperature >= uhot_band) {
    temperatureBand = 7;
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);  //This LED will be flashed!
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<Inferno!>"));
    #endif
  }
  else if (temperature >= vhot_band and temperature < uhot_band) {
    temperatureBand = 6;
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<Melting!>"));
    #endif
  }
  else if (temperature >= red_band and temperature < vhot_band) {
    temperatureBand = 5;
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<RED>"));
    #endif
  }
  else if (temperature >= yellow_band and temperature < red_band) {
    temperatureBand = 4;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<YELLOW>"));
    #endif
  }
  else if (temperature >= green_band and temperature < yellow_band) {
    temperatureBand = 3;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<GREEN>"));
    #endif
  }
  else if (temperature >= blue_band and temperature < green_band) {
    temperatureBand = 2;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<BLUE>"));
    #endif
  }
  else if (temperature >= vcold_band and temperature < blue_band) {
    temperatureBand = 1;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);    
    digitalWrite(WHITE_LED, HIGH);
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<Ice Age!>"));
    #endif
  }
  else {
    temperatureBand = 0;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);    
    digitalWrite(WHITE_LED, HIGH);  //This LED will be flashed!
    #ifndef PLOTDATA
      Serial.println(FLASHSTR("<Deep Space!>"));
    #endif
  }
  #ifdef PLOTDATA
    Serial.print(temperatureBand);
  #endif
}

//EOF
