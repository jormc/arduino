/*	RGBLed.h - RGB Led Library - 2014 jormc.es
 *
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 *
 * 	See file LICENSE.txt for further informations on licensing terms.
 */
#ifndef _RGBLed_H_
#define _RGBLed_H_

#include "Arduino.h"

class RGBLed {

	public:
		#define RED_LED_PIN				9
		#define GREEN_LED_PIN			10
		#define BLUE_LED_PIN			11

		RGBLed ();
		RGBLed (bool commonAnnode);
		RGBLed (byte redPin, byte greenPin, byte bluePin, bool commonAnode);
		void setColor(byte red, byte green, byte blue);
		void switchRedLed();
		void switchGreenLed();
		void switchBlueLed();

	private:
		typedef struct {
			byte redValue;
			byte greenValue;
			byte blueValue;
		} ColorValues;

		typedef struct {
			byte redPin;
			byte greenPin;
			byte bluePin;
		} LedPins;

		ColorValues colorValues;
		LedPins ledPins;
		bool isCommonAnode;

};

#endif
