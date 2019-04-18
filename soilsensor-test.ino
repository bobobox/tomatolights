#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

volatile float tempC;
volatile float temp_cumulative = 0;
volatile uint16_t capread;
volatile int ss_reading_count = 0;

void setup() {
  Serial.begin(115200);

  Serial.println("seesaw Soil Sensor example!");
  
  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while(1);
  } else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }

  tempC = ss.getTemp();
  capread = ss.touchRead(0);

}

void loop() {
  tempC = ss.getTemp();
  capread = ss.touchRead(0);

  if (ss_reading_count++ == 200) {

      ss_reading_count = 0;

      Serial.print("Temperature: "); Serial.print(temp_cumulative/200); Serial.println("*C");
      Serial.print("Capacitive: "); Serial.println(capread);
      delay(100);
      temp_cumulative = 0;

  }
  temp_cumulative = temp_cumulative + tempC;

}
