/*
 * atxpi_005.cpp
 *
 *  Created on: 8/11/2014
 *      Author: Jordi
 */

#include <Arduino.h>
#include <RGBLed.h>

#define POWER_BUTTON_INTERRUPT	0
#define RASPI_BOOT_INTERRUPT	1

#define POWER_BUTTON_PIN		2	// Interrupt 0 input
#define RASPI_BOOT_PIN			3	// Interrupt 1 input

#define POWER_RELAY_PIN			4
#define FANS_RELAY_PIN			5
#define RASPI_RESET_BUTTON		6
#define RASPI_RESET_SIGNAL_PIN	7

#define POWER_BUTTON_PUSH_TIME_TO_SHUTDOWN	3000
#define MAX_RASPI_BOOT_TIME					5000
#define MAX_RASPI_SHUTODOWN_TIME			5000

typedef enum ButtonStatus {
	PRESSED,
	RELEASED,
	IDDLE
};

typedef enum SystemStatus {
	SYS_OFF,
	SYS_STARTING,
	SYS_STARTING_ERROR,
	SYS_STARTED,
	SYS_SHUTTING_DOWN,
	SYS_SHUTTING_DOWN_ERROR,
	SYS_SHUTDOWN,
	SYS_FORCE_SHUTDOWN
};

typedef enum RaspiStatus {
	RPI_OFF,
	RPI_BOOTING,
	RPI_BOOTING_ERROR,
	RPI_BOOTED,
	RPI_RESETTING,
	RPI_RESETTING_ERROR,
	RPI_SHUTTING_DOWN
};

typedef struct {
	RGBLed statusLed;
	ButtonStatus powerButtonStatus;
	SystemStatus systemStatus;
	RaspiStatus raspiStatus;
	long powerButtonTime;
	long raspiBootTime;
	long raspiShutdownTime;
	long raspiResetTime;
	bool forceSystemShutdownSequenceStarted;
} System;
System system;

void setup() {
	// System Status Setup
	system.statusLed = new RGBLed();
	system.powerButtonStatus = IDDLE;
	system.systemStatus = SYS_OFF;
	system.raspiStatus = RPI_OFF;
	system.powerButtonTime = 0;
	system.raspiBootTime = 0;
	system.raspiShutdownTime = 0;
	system.raspiResetTime = 0;
	system.forceSystemShutdownSequenceStarted = false;

	// Power Button Setup
	pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(POWER_BUTTON_INTERRUPT, powerButtonStatusChanged, CHANGE);

	// Setup System Power Relay
	pinMode(POWER_RELAY_PIN, OUTPUT);
	switchOffPowerSystemRelay();

	// Setup Raspberry Pi Status
	pinMode(RASPI_BOOT_PIN, INPUT_PULLUP);
	attachInterrupt(RASPI_BOOT_INTERRUPT, piBootSignalStatusChanged, CHANGE);

	// Setup Raspberry Pi Reset System
	pinMode(RASPI_RESET_BUTTON, INPUT_PULLUP);
	pinMode(RASPI_RESET_SIGNAL_PIN, OUTPUT);
	digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);
}

void loop() {

	switch(system.systemStatus) {
		case SYS_OFF:
			switchToSystemOffStatus();
			if (system.powerButtonStatus == PRESSED) {
				system.powerButtonStatus = IDDLE;
				system.systemStatus = SYS_STARTING;
				system.raspiBootTime = millis();
			}
			break;

		case SYS_STARTING:
			switchToSystemStartingStatus();
			if (system.raspiStatus == RPI_BOOTED) {
				system.systemStatus = SYS_STARTED;
			} else if (millis() - system.raspiBootTime >= MAX_RASPI_BOOT_TIME) {
				system.raspiStatus = RPI_BOOTING_ERROR;
				system.systemStatus = SYS_STARTING_ERROR;
			}
			break;

		case SYS_STARTING_ERROR:
			switchToSystemStartingErrorStatus();
			if (system.powerButtonStatus == PRESSED) {
				system.powerButtonStatus = IDDLE;
				system.systemStatus = SYS_OFF;
			}
			break;

		case SYS_STARTED:
			switchToSystemStartedStatus();
			if (system.raspiStatus == RPI_OFF) {
				// Raspberry Pi has gone down...
				// It could be two things, a reset or a power down sequence...
				system.systemStatus = SYS_SHUTTING_DOWN;
				system.raspiStatus = RPI_SHUTTING_DOWN;
				system.raspiShutdownTime = millis();
			} else if (readResetButton()) {
				sendResetSequenceToRaspberryPi();
			} else if (system.powerButtonStatus == PRESSED) {
				system.systemStatus = SYS_FORCE_SHUTDOWN;
				system.powerButtonTime = millis();
			}
			break;

		case SYS_SHUTTING_DOWN:
			switchToSystemShuttingdownStatus();
			// Raspberry Pi is shutting down now...
			// It could be a halt or reset sequence
			// So we need to wait to see her status after a little time...
			if (system.raspiStatus == RPI_BOOTED) {
				// Raspberry Pi has been already booted
				// Perhaps she was making a reset? :-)
				system.systemStatus = SYS_STARTED;
			} else {
				if (millis() - system.raspiShutdownTime >= MAX_RASPI_SHUTODOWN_TIME) {
					// The Raspberry Pi has gone down
					// So we can Shutdown clompletly the system
					system.systemStatus = SYS_OFF;
				}
			}
			break;

		case SYS_FORCE_SHUTDOWN:
			if (system.powerButtonStatus == PRESSED) {
				if (millis() - system.powerButtonTime >= POWER_BUTTON_PUSH_TIME_TO_SHUTDOWN) {
					system.powerButtonStatus = IDDLE;
					system.systemStatus = SYS_OFF;
					system.powerButtonTime = 0;
				}
			} else if (system.powerButtonStatus == RELEASED) {
				system.powerButtonStatus = IDDLE;
				system.systemStatus = SYS_SHUTDOWN;
				system.powerButtonTime = 0;
				system.raspiShutdownTime = millis();
				sendShutdownSequenceToRaspberryPi();
			}
			break;

		case SYS_SHUTDOWN:
			switchToSystemShuttingdownStatus();
			if (system.raspiStatus == RPI_BOOTED) {
				if (millis() - system.raspiShutdownTime >= MAX_RASPI_SHUTODOWN_TIME) {
					system.systemStatus = SYS_SHUTTING_DOWN_ERROR;
				}
			} else {
				system.systemStatus = SYS_OFF;
			}
			break;

		case SYS_SHUTTING_DOWN_ERROR:
			switchToSystemShuttingdownErrorStatus();
			if (system.powerButtonStatus == PRESSED) {
				system.powerButtonStatus = IDDLE;
				system.systemStatus = SYS_OFF;
			}
			break;
	}
}

void powerButtonStatusChanged() {
	if (!digitalRead(POWER_BUTTON_PIN) == HIGH) {
		system.powerButtonStatus = PRESSED;
	} else {
		system.powerButtonStatus = RELEASED;
	}
}

void piBootSignalStatusChanged() {
	if (!digitalRead(RASPI_BOOT_PIN)) {
		system.raspiStatus = RPI_BOOTED;
	} else {
		system.raspiStatus = RPI_OFF;
	}
}

void switchToSystemOffStatus() {
	system.statusLed.switchRedLed();
	switchOffPowerSystemRelay();
}

void switchToSystemStartingStatus() {
	system.statusLed.switchBlueLed();
	switchOnPowerSystemRelay();
}

void switchToSystemStartingErrorStatus() {
	system.statusLed.setColor(255, 102, 000);	// Orange
}

void switchToSystemStartedStatus() {
	system.statusLed.switchGreenLed();
}

void switchToSystemShuttingdownStatus() {
	system.statusLed.switchBlueLed();
}

void switchToSystemShuttingdownErrorStatus() {
	system.statusLed.setColor(255, 102, 000);	// Orange
}

void switchOffPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	HIGH);
}

void switchOnPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	LOW);
}

int readResetButton() {
	return !digitalRead(RASPI_RESET_BUTTON);
}

void sendResetSequenceToRaspberryPi() {
	for (int i=0;i<5;i++) {
		digitalWrite(RASPI_RESET_SIGNAL_PIN, HIGH);
		delay (500);
		digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);
		delay (500);
	}
}

void sendShutdownSequenceToRaspberryPi() {
	for (int i=0;i<5;i++) {
		digitalWrite(RASPI_RESET_SIGNAL_PIN, HIGH);
		delay (100);
		digitalWrite(RASPI_RESET_SIGNAL_PIN, LOW);
		delay (100);
	}
}
