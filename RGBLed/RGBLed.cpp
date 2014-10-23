#include "RGBLed.h"

/*
 * Init RGBLed with the analogue pins for an annode-common rgb led type
 */
RGBLed::RGBLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin) {
	new RGBLed(redPin, greenPin, bluePin, false);
}

/*
 * Init RGBLed with the analogue pins for an annode-common or
 * cathode-common rgb led type. You must decide it with the commonCathode boolean param
 */
RGBLed::RGBLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin, bool commonCathode) {
	ledPins = {
		redPin,		// Output pin for Red Led
		greenPin,	// Output pin for Green Led
		bluePin		// Output pin for Blue Led
	};

	ledValues = {
		0,	// 0 value for Red tone
		0, 	// 0 value for Green tone
		0	// 0 value for Blue tone
	};

	isCommonCathode = commonCathode;
}

/**
 * Switch the LED to an specific color, determinater by three
 * RGB values, all them beetween 0 and 255.
 */
void RGBLed::switchColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue){
	ledValues = {
		redValue,
		greenValue,
		blueValue
	};

	if (isCommonCathode) {
		analogWrite(ledPins.redPin, 	ledValues.redValue);
		analogWrite(ledPins.greenPin, 	ledValues.greenValue);
		analogWrite(ledPins.bluePin, 	ledValues.blueValue);
	} else {
		analogWrite(ledPins.redPin, 	255 - ledValues.redValue);
		analogWrite(ledPins.greenPin, 	255 - ledValues.greenValue);
		analogWrite(ledPins.bluePin, 	255 - ledValues.blueValue);
	}
}

/*
 * Switch led to the Red color
 */
void RGBLed::switchRed() {
	switchColor(255, 0, 0);
}

/*
 * Switch led to the Green color
 */
void RGBLed::switchGreen() {
	switchColor(0, 255, 0);
}

/*
 * Switch led to the Blue color
 */
void RGBLed::switchBlue() {
	switchColor(0, 0, 255);
}
