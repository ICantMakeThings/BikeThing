/*
ppm - 111
gps tx - 008
gps rx - 006
button on switch side - 104
button on flat side - 107
light sensor - 031
reed switch - 024
ext. Media-button - 011
battery % - P0.04 (AIN2)
bmi160 - i2c
ds3231 - i2c
bme - i2c
display backlight - 100
display cs - 115
display dc - 017
display rst - 020
display busy - 022
sda - 102
scl - 101
sck - 002
mosi - 029
tx - 006
rx - 008

Using "promicro nrf52840" aka Nice!Nano v1 clone

https://github.com/ICantMakeThings/Nicenano-NRF52-Supermini-PlatformIO-Support
*/





#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <bluefruit.h> 
#include "RTClib.h"

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

#define CS_PIN   PIN_115
#define DC_PIN   PIN_017
#define RST_PIN  PIN_020
#define BUSY_PIN PIN_022

#define LIGHT PIN_031
//SWITCH L / R / EXTERNAL
#define SWL PIN_104
#define SWR PIN_107
#define SWEXT PIN_011 //aka media button

#define WHEELSENCE PIN_024 //reed switch

#define bat PIN_004 // dont work - todo, try AIN2?

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_I2C_ADDRESS 0x76
Adafruit_BME280 bme; // I2C
unsigned long delayTime;

unsigned long lastPressTime = 0;
int clickCount = 0;
const int multiClickTime = 400;

BLEDis bledis;
BLEHidAdafruit blehid;

GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

void setup() {
  Serial.begin(9600);

  Bluefruit.begin();
  Bluefruit.setName("BT DEVICE");
  Bluefruit.Periph.setConnInterval(9, 16);
  Bluefruit.setTxPower(0);

  bledis.setManufacturer("nor-gate");
  bledis.setModel("ng-v1");
  bledis.begin();

  blehid.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();

  Serial.println("Adver,");
  Bluefruit.Advertising.start(0);




  unsigned status;

  pinMode(LIGHT, INPUT_PULLDOWN);


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("Time can be wrong. ");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  Wire.begin();
   //  if (!bme.begin(BME280_I2C_ADDRESS)) {
  // Serial.println("Could not find a valid BME280 sensor, check wiring!");
 //   while (1);
//}
  rtc.begin();
  bme.begin(BME280_I2C_ADDRESS);

  //status = bme.begin();  
  
  //analogReadResolution(10); // 10-bit ADC
  
  SPI.begin(); //SPI.begin(SCK_PIN, -1, MOSI_PIN);
  display.init();
  display.setRotation(0);
  display.setPartialWindow(0, 0, 290, 250);

  // Show a loading screen on the display
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Hello");
  } while (display.nextPage()); //todo - add qrcode to link git when powered off & welcome screen.
}

void loop() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  int speed = random(20, 100); //   < < < Placeholder
  DateTime now = rtc.now();

  // Update display
  display.setPartialWindow(0, 0, 290, 250);
  display.firstPage();
  do {
    display.setPartialWindow(0, 0, 290, 250); // Update the area containing the text
    display.fillRect(0, 0, 290, 250, GxEPD_WHITE);  // Clear the area


    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(2);
    display.setCursor(40, 10);
    display.print(now.hour(), DEC);

    display.setCursor(60, 10);
    display.print(':');
    
    display.setCursor(70, 10);
    display.print(now.minute(), DEC);


    // Speed Display
    display.setTextSize(2);
    display.setCursor(50, 50);
    display.print("Speed:");
    display.setCursor(140, 50);
    display.print(speed);
    display.print(" km/h");

    // Divider Line
    display.drawLine(10, 100, 280, 100, GxEPD_BLACK);

    display.setTextSize(1);
    display.setCursor(10, 120);
    display.print("Temp: ");
    display.print(temperature, 1);
    display.print(" C");

    display.setCursor(10, 150);
    display.print("Humidity: ");
    display.print(humidity, 1);
    display.print(" %");

    display.setCursor(10, 180);
    display.print("Pressure: ");
    display.print(pressure, 1);
    display.print(" hPa");

  } while (display.nextPage());
/*
  if (digitalRead(SWEXT) == LOW) {
    unsigned long pressTime = millis();
    
    if (pressTime - lastPressTime > multiClickTime) {
      clickCount = 1;
    } else {
      clickCount++;
    }

    lastPressTime = pressTime;

    while (digitalRead(SWEXT) == LOW);
    delay(50);
  }

  if (millis() - lastPressTime > multiClickTime && clickCount > 0) {
    if (clickCount == 1) {
      Serial.println("Pause");
      blehid.consumerKeyPress(HID_USAGE_CONSUMER_PLAY_PAUSE);
      delay(100);
      blehid.consumerKeyRelease();
    } else if (clickCount == 2) {
      Serial.println("Next Track");
      blehid.consumerKeyPress(HID_USAGE_CONSUMER_SCAN_NEXT);
      delay(100);
      blehid.consumerKeyRelease();
    } else if (clickCount == 3) {
      Serial.println("Previous Track");
      blehid.consumerKeyPress(HID_USAGE_CONSUMER_SCAN_PREVIOUS);
      delay(100);
      blehid.consumerKeyRelease();
    }

    clickCount = 0;
  }
*/
  delay(100);
}