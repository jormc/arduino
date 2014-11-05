/*	ATXPi v0.1b - jormc.es 2014-11-03
 *
 * 	ATXPi is a power control system for Raspberry Pi controlled by Arduino and based
 * 	on the function of existing ATX power supplies. The system is continuously monitoring
 * 	the state of the Raspberry Pi using interrupts, so it can detect at any time if the Pi
 * 	is connected or not (booted or not). Depending on its state, the system will interrupt
 * 	or allow the power supplied by an active switch (currently, a test output relay).
 *
 *  This is the system functions (the code is commented):
 *
 *  1. If the system is OFF and we press Power Button, the system goes UP (it is waiting
 *     for a externa input interrupt connected to the button).
 * 	2. If the system is going up, then it closes the electronic switch (actually a relay)
 *     and suplies the power to the Pi power input.
 * 	3. During the system starting status, it waits for Pi to get booted correctly. For this
 *     reason, the system waits for a interrupt that flags that the Pi has been booted correctly.
 * 	4. If the external input interrupt says that Pi is booted correctly, the system enters
 *     in a UP status. But if any error appears during this process and the Pi has not
 *     booted into a determinated time, the system detects an error and then inform the
 *     user that the Pi is not booted correctly. In this case, the user can press the power
 *     button and turn off completly the system.
 *  5. Once the system is UP, there can occurs two things:
 *     5.1 The Pi shuts down suddenly. The system regonizes this state and inform to the
 *         user by diferent led colors. Then the user can press Power Button and turn off
 *         the system completly.
 *     5.2 The Pi has been booted correctly, and we want to turn off the system. Then, we
 *     	   must turn off and halt the Pi completly, and the system will recognize this status.
 *     	   Then the system goes into a controlled shut down and turn OFF all it.
 *  6. Finally, if there is a problem getting down the Pi, the system waits for a maximum
 *     shut down time and, if we've reached it, the system goes into an Shut Down Error Status
 *     and inform us likely point 4. Then the user can press Power Button and turn OFF all
 *     the system.
 *
 *  CAUTION: this is a experimental and beta version, so se at your own risk!!!
 *
 *  Errors, oppinions, reviews and other messages are welcome!
 *
 *  I hope you enjoy it!
 *
 */
#include "ATXPi.h"

#define POWER_BUTTON_INTERRUPT	0			// Interrupt 0
#define RASPI_BOOT_INTERRUPT	1			// Interrupt 1

#define POWER_BUTTON_PIN		2			// Power Button input pin
#define RASPI_BOOT_PIN			3			// Raspberry Pi boot status input pin
#define POWER_RELAY_PIN			4			// Power Relay Control output pin
#define FANS_RELAY_PIN			5			// Not used at this moment

#define COMMON_ANODE						// Inidacate that we use a common anode RGB led
#define RED_LED_PIN				9			// Red led output pin
#define GREEN_LED_PIN			10			// Green led output pin
#define BLUE_LED_PIN			11			// Blue led output pin

#define MAX_POWER_BTN_TIMER		3000		// Not used
#define MAX_RASPI_BOOT_TIME		5000		// Max time to wait for Pi boot process completes
#define MAX_RASPI_DOWN_TIME		5000		// Max time to wait for Pi shutdown process completes

// Button Status
enum ButtonStatus {
	PRESSED,
	RELEASED,
	IDDLE
};

// Power System Status (Arduino)
enum SystemStatus {
	OFF,
	STARTING,
	STARTING_ERROR,
	STARTED,
	SHUTTING_DOWN,
	SHUTTING_DOWN_ERROR,
	FORCE_SHUTDOWN
};

// Raspberry Pi boot process status
enum RaspiStatus {
	DOWN,
	BOOTING,
	BOOTING_ERROR,
	BOOTED,
	GOING_DOWN,
	GOING_DOWN_ERROR
};

volatile ButtonStatus powerButtonStatus = IDDLE;	// Unknow
volatile SystemStatus systemStatus = OFF;			// System is off at start
volatile RaspiStatus raspiStatus = DOWN;			// Raspberry Pi power is down at start
volatile unsigned int piBootStatus = LOW;			// Raspberry Pi boot is not present
volatile unsigned long powerButtonTimer = 0;		// Not used
volatile unsigned long raspiBoottingTimer = 0;		// Timer for Raspberry Pi boot time
volatile unsigned long raspiGoingDownTimer = 0;		// Timer for Raspberry Pi shutdown time

void setup() {
	// Only for test!
	Serial.begin(9600);

	// Setup all system components
	systemSetup();
	setupLeds();
	setupPowerButton();
	setupSystemPowerRelay();
	setupSystemFansRelay();
	setupRaspiStatus();
}

void loop() {
	switch (systemStatus) {
		case OFF:
			switchToOffStatus();
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				systemStatus = STARTING;
				raspiBoottingTimer = millis();

				Serial.println("Power Button Pressed, let start the system...");
			}
			break;

		case STARTING:
			switchToStartingStatus();
			readRaspiBootStatus();
			switch(raspiStatus) {
				case BOOTED:
					systemStatus = STARTED;
					Serial.println("Raspberry Pi has been booted!");
					break;

				case BOOTING_ERROR:
					systemStatus = STARTING_ERROR;
					Serial.println("There was an error with Raspberry Pi's boot! The max boot time has been reached!");
					break;

				default:	// Raspi is booting
					break;
			}
			break;

		case STARTING_ERROR:
			switchToStartingErrorStatus();
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				systemStatus = FORCE_SHUTDOWN;
				raspiGoingDownTimer = millis();

				Serial.println("Let's go down system, Raspberry Pi wasn't booted correctly...");
			}
			break;

		case STARTED:
			switchToStartedStatus();
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				systemStatus = SHUTTING_DOWN;
				raspiStatus = GOING_DOWN;
				raspiGoingDownTimer = millis();

				Serial.println("Let's go down the system, we must call Raspberry Pi to goes down too!");
			}
			break;

		case SHUTTING_DOWN:
			switchToShuttingDownStatus();
			readRaspiBootStatus();
			switch(raspiStatus) {
				case DOWN:
					systemStatus = OFF;

					Serial.println("The system status is OFF");
					break;

				case GOING_DOWN_ERROR:
					systemStatus = SHUTTING_DOWN_ERROR;

					Serial.println("The system status is Shutting Down Error");
					break;

				default:	// Raspi is going down
					break;
			}
			break;

		case SHUTTING_DOWN_ERROR:
			switchToShuttingDownErrorStatus();
			if (powerButtonStatus == PRESSED) {
				powerButtonStatus = IDDLE;
				systemStatus = FORCE_SHUTDOWN;
				raspiGoingDownTimer = millis();

				Serial.println("Let's go down system, Raspberry Pi wasn't shutted down correctly...");
			}
			break;

		case FORCE_SHUTDOWN:
			switchToOffStatus();
			break;

		default:

			break;
	}

}

////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////
void systemSetup() {

}

void setupLeds() {

}

void setupPowerButton() {
	powerButtonStatus = IDDLE;
	pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(POWER_BUTTON_INTERRUPT, powerButtonStatusChanged, CHANGE);
}

void powerButtonStatusChanged() {
	if (!digitalRead(POWER_BUTTON_PIN) == HIGH) {
		powerButtonStatus = PRESSED;
	} else {
		powerButtonStatus = RELEASED;
	}
}

void setupSystemPowerRelay() {
	pinMode(POWER_RELAY_PIN, OUTPUT);
	switchOffPowerSystemRelay();
}

void setupSystemFansRelay() {

}

void setupRaspiStatus() {
	raspiStatus = DOWN;

	// We'll use a input interrupt to test if Raspberry Pi is booted
	piBootStatus = LOW;
	pinMode(RASPI_BOOT_PIN, INPUT_PULLUP);
	attachInterrupt(RASPI_BOOT_INTERRUPT, piBootStatusChanged, CHANGE);
}

void piBootStatusChanged() {
	// When Pi has ben Booted, the pin is HIGH
	piBootStatus = !digitalRead(RASPI_BOOT_PIN);
	if (piBootStatus) {
		Serial.println("Raspberry Pi Interrupt: BOOTED!");
	} else {
		Serial.println("Raspberry Pi Interrupt: HALTED!");
	}
}

////////////////////////////////////////////////////////////////////////////
// SYSTEM CONTROL
////////////////////////////////////////////////////////////////////////////
void switchToOffStatus() {
	showSystemOffStatus();
	switchOffPowerSystemRelay();
	systemStatus = OFF;
	raspiStatus = DOWN;
}

void showSystemOffStatus() {
	switchRedLed();
}

void switchToStartingStatus() {
	showSystemStartingStatus();
	switchOnPowerSystemRelay();
	raspiStatus = BOOTING;
}

void showSystemStartingStatus() {
	switchBlueLed();
}

void showSystemStartingErrorStatus() {
	// Orange
	setColor(255, 102, 0);
}

void switchToStartingErrorStatus() {
	showSystemStartingErrorStatus();
}

void switchToStartedStatus() {
	showSystemStartedStatus();
}

void showSystemStartedStatus() {
	switchGreenLed();
}

void switchToShuttingDownErrorStatus() {
	showSystemShuttingDownErrorStatus();
}

void showSystemShuttingDownErrorStatus() {
	// Orange
	setColor(255, 102, 0);
}

void switchToShuttingDownStatus() {
	showSystemShuttingDownStatus();
}

void showSystemShuttingDownStatus() {
	switchBlueLed();
}

////////////////////////////////////////////////////////////////////////////
// RELAYS CONTROL
////////////////////////////////////////////////////////////////////////////
void switchOffPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	HIGH);
}

void switchOnPowerSystemRelay() {
	digitalWrite(POWER_RELAY_PIN, 	LOW);
}

////////////////////////////////////////////////////////////////////////////
// RASPBERRY PI CONTROL - POWER RELAY ASSOCIATTED
////////////////////////////////////////////////////////////////////////////
// Read from GPIO output if Raspi is booted up
void readRaspiBootStatus() {
	if (raspiStatus == BOOTING) {
		// Raspberry Pi is booting
		if (millis() - raspiBoottingTimer >= MAX_RASPI_BOOT_TIME) {
			raspiStatus = BOOTING_ERROR;
		} else {
			// Look at RASPI_BOOT_PIN if its status has changed
			// A HIGH value tell us that Raspbery Pi is BOOTED
			if (piBootStatus == HIGH) {
				raspiStatus = BOOTED;
			} else {
				raspiStatus = BOOTING;
			}
		}
	} else if (raspiStatus == GOING_DOWN) {
		// Raspberry Pi is going down
		if (millis() - raspiGoingDownTimer >= MAX_RASPI_DOWN_TIME) {
			raspiStatus = GOING_DOWN_ERROR;
		} else {
			// Look at RASPI_BOOT_PIN if its status has changed
			// A DOWN value tell us that Raspbery Pi is DOWN
			if (piBootStatus == LOW) {
				raspiStatus = DOWN;
			} else {
				raspiStatus = GOING_DOWN;
			}
		}
	} else {
		Serial.println("This is a strange Raspberry Pi Boot status... Uugh!!! :-(");
	}
}

////////////////////////////////////////////////////////////////////////////
// RGB LED CONTROL
////////////////////////////////////////////////////////////////////////////
void setColor(int red, int green, int blue) {
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif

  analogWrite(RED_LED_PIN, red);
  analogWrite(GREEN_LED_PIN, green);
  analogWrite(BLUE_LED_PIN, blue);
}

void switchRedLed() {
	setColor(255,0,0);
}

void switchGreenLed() {
	setColor(0,255,0);
}

void switchBlueLed() {
	setColor(0,0,255);
}
