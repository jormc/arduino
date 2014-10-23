/*
 * RGB Led Library
 *
 * Language Library to handle basic Arduino RGB LEDs easily.
 * Sets the LED and switch pins in basic colors , or even apply
 * a mapping RGB values ​​from 0 to 255 for each of the 3 components.
 */
#ifndef _RGBLed_H_
#define _RGBLed_H_

#include "Arduino.h"

class RGBLed {

private:
	struct LedPins {
		uint8_t redPin;
		uint8_t greenPin;
		uint8_t bluePin;
	};

	struct LedValues {
		uint8_t redValue;
		uint8_t greenValue;
		uint8_t blueValue;
	};

	bool isCommonCathode;
	LedPins ledPins;
	LedValues ledValues;

public:
	RGBLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
	RGBLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin, bool commonCathode);

	void switchRed();
	void switchGreen();
	void switchBlue();
	void switchColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);

protected:

};

#endif
