/*
 * SwitchBaseColors.ino - jormc.es (2014)
 *
 * Basic example that switches between all three
 * basic RGB colors every secod using the RGBLed library
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
  led.switchRedLed();
  delay(1000);
  led.switchGreenLed();
  delay(1000);
  led.switchBlueLed();
  delay(1000);
}
