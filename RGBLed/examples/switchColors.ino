/*
 * 	Basic example of RGBLed Library from jormc.es
 *
 * 	Initialize a new RGBLed object with 4, 5 and 6 analog pins,
 * 	and switch beetween Red, Green and Blue colors every second.
 *
 * 	By default the LED is an Annode-Common type. If you want to use
 * 	the Cathode-Common type, you must use the commented constructor
 */
#include <RGBLed.h>

// Analogue Output Pins
#define redLed 		4
#define greenLed 	5
#define blueLed 	6

// Init a RGBLed object with the arduino pins
// for an Annode Common Led type
RGBLed led(redLed, greenLed, blueLed);

// Or for an Cathode Common Led type
// RGBLed led(redLed, greenLed, blueLed, true);

void setup() {

}

void loop() {
	// Turn on red led
	led.switchRed();
	delay(1000);

	// Turn on green led
	led.switchGreen();
	delay(1000);

	// Turn on blue led
	led.switchBlue();
	delay(1000);
}
