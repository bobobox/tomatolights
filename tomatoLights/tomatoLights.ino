#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include "Adafruit_seesaw.h"
#include "wifiCreds.h"


// Light timing
static int onHour = 5;
static int offHour = 23;

// Light Duty cycle, from 0-1023 where 0 is off an 1023 is 100% on
static int dutyCycle = 1023;

// Light GPIO pins
volatile byte lightState = 0;
static byte lightPin = 12;

// OLED setup
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

// Time setup, for Central Daylight Time. Won't bother with CST for now.
const long utcOffsetInSeconds = -18000;
const long updateIntervalMillis = 300 * 1000; // Only actually talk to NTP server every 5 minutes.

volatile int lastNtpSyncHour = 99;
volatile int currentHour;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, updateIntervalMillis);

// Seesaw sensor for humidity and temperature
Adafruit_seesaw ss;

volatile float tempCumulative = 0;
volatile float tempAvg = 0;
// Account for crappy temperature sensor in SeeSaw
static float tempCalibrationC = -2.2;
volatile uint16_t humCumulative = 0;
volatile uint16_t humAvg = 0;
volatile int ssReadingCount = 0;

// Humidity warning light
static int humWarningThreshold = 500;
static byte humWarningPin = 15;
volatile byte humWarningLightState = LOW;
volatile bool humWarning = false;


void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_profont11_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}


void setup() {

  pinMode(humWarningPin, OUTPUT);
  digitalWrite(humWarningPin, humWarningLightState);

  pinMode(lightPin, OUTPUT);
  // Stay dim until ready to go
  analogWrite(lightPin, 20);

  u8g2.begin();
  u8g2_prepare();
  
  u8g2.clearBuffer();
  u8g2.drawStr( 0, 0, "Connecting to WiFi...");
  u8g2.sendBuffer();

  WiFi.mode(WIFI_STA);
  //These are defined in the `creds.h` header file
  WiFi.begin(wifiSsid, wifiPassword);

  Serial.begin(115200);
  Serial.print( "Connecting to WiFi..." );
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 700 );
    Serial.print( "." );
  }
  Serial.print ( "WiFi Connected." );

  u8g2.clearBuffer();
  u8g2.drawStr( 0, 0, "WiFi Connected.");
  u8g2.sendBuffer();

  delay(1000);

  u8g2.clearBuffer();
  u8g2.drawStr( 0, 0, "Detecting Sensors...");
  u8g2.sendBuffer();
  delay(1000);
 
  if (!ss.begin(0x36)) {
    delay(700);
    Serial.println("ERROR! seesaw not found");
    while(1);
  } else {
    Serial.print("SeeSaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
    u8g2.clearBuffer();
    u8g2.drawStr( 0, 0, "Sensors found.");
    u8g2.sendBuffer();
  }

  delay(1000);

  timeClient.begin();

}

void loop() {

  // This won't actually update except every updateIntervalMillis
  timeClient.update();

  currentHour = timeClient.getHours();

  u8g2.clearBuffer();

  u8g2.setCursor(0, 0);
  u8g2.print(timeClient.getFormattedTime());
  u8g2.print(" CDT");

  u8g2.setCursor(0, 11);
  u8g2.print("Lights on ");
  if (onHour < 10) {
    u8g2.print("0");
  }
//  u8g2.setFont(u8g2_font_m2icon_7_tf);
  u8g2.print(onHour);
  u8g2.print(":00-");

  if (offHour < 10) {
    u8g2.print("0");
  }
  u8g2.print(offHour);
  u8g2.print(":00");
 
  u8g2.setCursor(83, 0);
  u8g2.print("DC ");
  u8g2.print(100 * dutyCycle / 1023);
  u8g2.print("%");
  
  u8g2.setCursor(0, 22);
  u8g2.print("Hum ");
  // Produce a vaguely 1-10 scale of humidity, given that readings seem to go
  // between 300 and 700-ish in soil.
  u8g2.print((humAvg - 200) / 55);
  u8g2.print("/10");
  u8g2.print("   Temp ");
  u8g2.print(static_cast<int>(round(tempAvg * 1.8 + 32)));
  //u8g2.print("°C");
  u8g2.print("\xb0" "F");

  u8g2.sendBuffer();

  if (ssReadingCount++ == 10) {

    tempAvg = tempCumulative/ssReadingCount;
    tempAvg = tempAvg + tempCalibrationC;
    humAvg = humCumulative/ssReadingCount;

    if (humAvg < humWarningThreshold) {
        humWarning = true;
    } else {
        humWarning = false;
    }

    ssReadingCount = 0;
    tempCumulative = 0;
    humCumulative = 0;

  }
    
  Serial.println(ss.getTemp());
  Serial.println(ss.touchRead(0));

  // Temp and humidity readings
  tempCumulative = tempCumulative + ss.getTemp();
  humCumulative = humCumulative + ss.touchRead(0);

  if (humWarning) {

    if (humWarningLightState == LOW) {

      humWarningLightState = HIGH;

    } else {

      humWarningLightState = LOW;

    }

  } else {

      humWarningLightState = LOW;

  }

  digitalWrite(humWarningPin, humWarningLightState);


  if (onHour < offHour) {

    if (currentHour >= onHour and currentHour < offHour) {
        
      // Lights on 
      analogWrite(lightPin, dutyCycle);

    } else {

      // Lights off 
      analogWrite(lightPin, 0);

    }

  } else { // This will catch offHour < onHour and onHour == offHour, which we'll treat as always on

    if (currentHour >= onHour or currentHour < offHour) {
        
      // Lights on 
      analogWrite(lightPin, dutyCycle);

    } else {

      // Lights off 
      analogWrite(lightPin, 0);

    }

    
  }

  delay(1000);
}


