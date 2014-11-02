/*	RGBLed.cpp - RGB Led Library - 2014 jormc.es
 *
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 *
 * 	See file LICENSE.txt for further informations on licensing terms.
 */
#include "RGBLed.h"

// RGBLed default constructor, for a common anode led
// and led pins are represented by the default values (see LedPins struct)
RGBLed::RGBLed () {
	isCommonAnode = true;
	ledPins = {RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN};
}

// RGBLed constructor, where you can choose if the led is common anode
// and led pins are represented by the default values (see LedPins struct)
RGBLed::RGBLed (bool commonAnode) {
	isCommonAnode = commonAnode;
	ledPins = {RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN};
}

// RGBLed constructor, where you must determine the led pins
// and decide if it is a common anode led by the boolean parameter
RGBLed::RGBLed (byte redPin, byte greenPin, byte bluePin, bool commonAnode) {
	isCommonAnode = commonAnode;
	ledPins = {redPin, greenPin, bluePin};
}

// Switch to a color represented by a R, G, B byte group values (0 - 255)
void RGBLed::setColor(byte red, byte green, byte blue) {
	colorValues = {red, green, blue};

	if (isCommonAnode) {
		red = 255 - red;
		green = 255 - green;
		blue = 255 - blue;
	}

	analogWrite(ledPins.redPin, red);
	analogWrite(ledPins.greenPin, green);
	analogWrite(ledPins.bluePin, blue);
}

// Switch to Red pure color
void RGBLed::switchRedLed() {
	setColor(255, 0, 0);
}

// Switch to Green pure color
void RGBLed::switchGreenLed() {
	setColor(0, 255, 0);
}

// Switch to Blue pure color
void RGBLed::switchBlueLed() {
	setColor(0, 0, 255);
}
