/*
 * RGB Led Library
 *
 * A basic library for RGB Led control. You can use it for switch the led colors
 * beetween Red, Green and Blue, o a conjuntion of them, passing the RGB color in
 * numeric mode. You must use values beetween 0 and 255 for each color.
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
