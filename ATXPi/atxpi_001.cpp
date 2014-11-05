/*
 * atxpi_001.cpp
 *
 *  Created on: 5/11/2014
 *      Author: Jordi
 */

#include <Arduino.h>
#include <RGBLed.h>

#define POWER_BUTTON_INTERRUPT	0			// Interrupt 0
#define RASPI_BOOT_INTERRUPT	1			// Interrupt 1
#define POWER_BUTTON_PIN		2			// Power Button input pin
#define RASPI_BOOT_PIN			3			// Raspberry Pi boot status input pin
#define POWER_RELAY_PIN			4			// Power Relay Control output pin
#define FANS_RELAY_PIN			5			// Not used at this moment
#define RASPI_RESET_BUTTON		6			// Reset button
#define RASPI_RESET_SIGNAL_PIN	7			// Reset Signal Pin to send to Raspberry Pi

#define MAX_POWER_BUTTON_TIME	3000		// Max time to wait for a forced shutdown pressing the power button
#define MAX_RASPI_BOOT_TIME		5000		// Max time to wait for Pi boot process completes
#define MAX_RASPI_DOWN_TIME		5000		// Max time to wait for Pi shutdown process completes

typedef enum SystemStatus {
	SYSTEM_OFF,
	SYSTEM_STARTING,
	SYSTEM_STARTING_ERROR,
	SYSTEM_STARTED,
	SYSTEM_SHUTTING_DOWN,
	SYSTEM_SHUTTING_DOWN_ERROR,
	SYSTEM_FORCED_SHUTDOWN
};

typedef enum RaspberryPiStatus {
	RASPI_OFF,
	RASPI_BOOTING,
	RASPI_BOOTING_ERROR,
	RASPI_BOOTED,
	RASPI_SHUTTING_DOWN,
	RASPI_SHUTTING_DOWN_ERROR
};

typedef enum ButtonStatus {
	PRESSED,
	RELEASED,
	IDDLE
};
ButtonStatus powerButtonStatus;

typedef struct {
	SystemStatus systemStatus;
	RaspberryPiStatus raspiStatus;
	long powerButtonTime;
	long raspiBootingTime;
	long raspiShutdownTime;
} Status;
Status status;

RGBLed led;

void setup() {

	// Only for test!
	Serial.begin(9600);

	// Init System Status
	status.systemStatus = SYSTEM_OFF;
	status.raspiStatus = RASPI_OFF;
	status.raspiBootingTime = 0;
	status.raspiShutdownTime = 0;

	// Setup Power Button
	powerButtonStatus = IDDLE;
	pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(POWER_BUTTON_INTERRUPT, powerButtonStatusChanged, CHANGE);

	// Setup System Power Relay
	pinMode(POWER_RELAY_PIN, OUTPUT);
	switchOffPowerSystemRelay();

	// Setup Raspberry Pi Status
	pinMode(RASPI_BOOT_PIN, INPUT_PULLUP);
	attachInterrupt(RASPI_BOOT_INTERRUPT, piBootStatusChanged, CHANGE);

	// Setup Raspberry Pi Reset System
	pinMode(RASPI_RESET_BUTTON, INPUT_PULLUP);
	pinMode(RASPI_RESET_SIGNAL_PIN, OUTPUT);
	digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);

	// Setup the RGB Led
	led = new RGBLed();
}

void loop() {

	switch(status.systemStatus) {
		case SYSTEM_OFF:
			led.switchRedLed();
			switchOffPowerSystemRelay();
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				status.raspiStatus = RASPI_OFF;
				status.systemStatus = SYSTEM_STARTING;
			}
			break;

		case SYSTEM_STARTING:
			led.switchBlueLed();
			switchOnPowerSystemRelay();
			switch(status.raspiStatus) {
				case RASPI_OFF:
					status.raspiStatus = RASPI_BOOTING;
					status.raspiBootingTime = millis();
					break;

				case RASPI_BOOTING:
					if (millis() - status.raspiBootingTime >= MAX_RASPI_BOOT_TIME) {
						status.raspiStatus = RASPI_BOOTING_ERROR;
					}
					break;

				case RASPI_BOOTING_ERROR:
					status.systemStatus = SYSTEM_STARTING_ERROR;
					break;

				case RASPI_BOOTED:
					status.systemStatus = SYSTEM_STARTED;
					break;
			}
			break;

		case SYSTEM_STARTING_ERROR:
			led.setColor(255, 102, 000);
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				status.systemStatus = SYSTEM_OFF;
			}
			if (status.raspiStatus == RASPI_BOOTED) {
				status.systemStatus = SYSTEM_STARTED;
			}
			break;

		case SYSTEM_STARTED:
			led.switchGreenLed();
			if (readResetButton()) {
				digitalWrite(RASPI_RESET_SIGNAL_PIN, HIGH);
				delay(100);
				digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);
			}

			if(status.raspiStatus == RASPI_SHUTTING_DOWN) {
				status.raspiShutdownTime = millis();
				status.systemStatus = SYSTEM_SHUTTING_DOWN;
			}

			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				status.powerButtonTime = millis();
				status.systemStatus = SYSTEM_FORCED_SHUTDOWN;
			}

			break;

		case SYSTEM_SHUTTING_DOWN:
			led.switchBlueLed();
			switch(status.raspiStatus) {
				case RASPI_SHUTTING_DOWN:
					if (millis() - status.raspiShutdownTime >= MAX_RASPI_DOWN_TIME) {
						status.raspiStatus = RASPI_OFF;
					}
				break;

				case RASPI_OFF:
					status.systemStatus = SYSTEM_OFF;
					break;
			}
			break;

		case SYSTEM_FORCED_SHUTDOWN:
			if (powerButtonStatus == IDDLE) {
				if (millis() - status.powerButtonTime >= MAX_POWER_BUTTON_TIME) {
					status.systemStatus = SYSTEM_OFF;
					status.raspiStatus = RASPI_OFF;
				}
			} else if (powerButtonStatus == RELEASED) {
				status.systemStatus = SYSTEM_STARTED;
			}
			break;
	}

}

void powerButtonStatusChanged() {
	if (!digitalRead(POWER_BUTTON_PIN) == HIGH) {
		powerButtonStatus = PRESSED;
	} else {
		powerButtonStatus = RELEASED;
	}
}

void piBootStatusChanged() {
	if (!digitalRead(RASPI_BOOT_PIN)) {
		status.raspiStatus = RASPI_BOOTED;
	} else {
		status.raspiStatus = RASPI_SHUTTING_DOWN;
	}
}

void switchOffPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	HIGH);
}

void switchOnPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	LOW);
}

int readResetButton() {
	if (!digitalRead(RASPI_RESET_BUTTON)) {
		digitalWrite(RASPI_RESET_SIGNAL_PIN, HIGH);
		return HIGH;
	} else {
		digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);
		return LOW;
	}
}
