/*
 * SwitchMoreColors.ino - jormc.es (2014)
 *
 * Basic example that switches between most other colors
 * that basic RGB ones every secod using the RGBLed library
 *
 * For this example, we assume that we use:
 * 	- RGB Common Anode Led
 * 	- RGB Led Digital Pins are R=9, G=10, B=11
 */
#include <RGBLed.h>

RGBLed led;
void setup() {
  led = new RGBLed();
}

void loop() {
  led.setColor(255, 102, 0);	// Orange
  delay(1000);
  led.setColor(255, 204, 0);	// Some Yellow
  delay(1000);
  led.setColor(51, 102, 153);	// Likely Gray Blued Color...
  delay(1000);
}
