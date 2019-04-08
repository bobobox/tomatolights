# tomatolights

LED grow light timer for my tomato seedlings.

![Scrappy Initial Setup](readme_images/scrappy_setup.jpg?raw=true "Scrappy Initial Setup")

Hardware:

 - ESP8266 microcontroller and OLED display: http://www.heltec.cn/project/wifi-kit-8/?lang=en
 - IRL3705N power mosfet for switching LEDS
 - 2.5A capable USB power supply
 - 5V LED grow lights - using this "40W" (LOL) USB powered unit from Amazon, replacing the timer/controller unit and power source: https://www.amazon.com/gp/product/B07H57K565/)

Requires creation of a `wifiCreds.h` file containing the WiFi SSID and password (example file `wifiCreds-example.h` provided).
