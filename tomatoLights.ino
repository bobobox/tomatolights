#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include "wifiCreds.h"

// Light timing

static int onHour = 5;
static int offHour = 23;

// Light Duty cycle, from 0-1023 where 0 is off an 1023 is 100% on
static int dutyCycle = 950;

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

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void setup(){

  pinMode(lightPin, OUTPUT);
  // Stay dim until ready to go
  analogWrite(lightPin, 20);

  u8g2.begin();
  u8g2_prepare();
  
  Serial.begin(115200);
  u8g2.clearBuffer();
  u8g2.drawStr( 0, 0, "Connecting to WiFi...");
  u8g2.sendBuffer();

  WiFi.mode(WIFI_STA);
  //These are defined in the `creds.h` header file
  WiFi.begin(wifiSsid, wifiPassword);
  Serial.print ( "Connecting to WiFi..." );
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 700 );
    Serial.print ( "." );
  }
  Serial.print ( "WiFi Connected." );
  u8g2.clearBuffer();
  u8g2.drawStr( 0, 0, "WiFi Connected.");
  u8g2.sendBuffer();

  delay(3000);

  u8g2.clearBuffer();

  u8g2.setCursor(0, 11);
  u8g2.print(" Lights on ");
  if (onHour < 10) {
    u8g2.print("0");
  }
  u8g2.print(onHour);
  u8g2.print(":00 CDT");

  u8g2.setCursor(0, 22);
  u8g2.print("Lights off ");
  if (offHour < 10) {
    u8g2.print("0");
  }
  u8g2.print(offHour);
  u8g2.print(":00 CDT");
 
  u8g2.setCursor(82, 0);
  u8g2.print("DS ");
  u8g2.print(100*dutyCycle/1023);
  u8g2.print("%");

  u8g2.sendBuffer();


  timeClient.begin();

}

void loop() {

  // This won't actually update except every updateIntervalMillis
  timeClient.update();

  currentHour = timeClient.getHours();

  u8g2.setCursor(0, 0);
  u8g2.print(timeClient.getFormattedTime());
  u8g2.print(" CDT");
  u8g2.sendBuffer();

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


