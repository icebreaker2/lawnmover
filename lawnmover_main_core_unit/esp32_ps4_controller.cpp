#include "ESP32_PS4_Controller.h"
#include <PS4Controller.h>
#include <serial_logger.h>

void toggle(const int pin) {
	if (digitalRead(pin) == 0) {
		digitalWrite(pin, HIGH);
	} else {
		digitalWrite(pin, LOW);
	}
}

ESP32_PS4_Controller::ESP32_PS4_Controller(const char *masterMac, Timer<> &timer, const unsigned long timerDelay,
										   const int readyPin, const int connectedPin, const int commandReceivedPin) :
		k_masterMac(masterMac), k_timerDelay(timerDelay), k_readyPin(), k_connectedPin(connectedPin),
		k_commandReceivedPin(commandReceivedPin) {

	// LEDs (all on initially)
	pinMode(k_readyPin, OUTPUT);
	pinMode(k_connectedPin, OUTPUT);
	pinMode(k_commandReceivedPin, OUTPUT);
	digitalWrite(k_readyPin, LOW);
	digitalWrite(k_connectedPin, LOW);
	digitalWrite(k_commandReceivedPin, LOW);

	SerialLogger::info("Beginning listening for master PS4 controller connection to: %s", k_masterMac);
	PS4.begin(masterMac);

	timer.every(k_timerDelay, [this](void *) -> bool {
		if (readState()) {
			if (!_connected) {
				digitalWrite(k_connectedPin, HIGH);
				SerialLogger::info("Connected to: %s", k_masterMac);
				_connected = true;
			}
		} else {
			if (_connected) {
				digitalWrite(k_connectedPin, LOW);
				digitalWrite(k_commandReceivedPin, LOW);
				SerialLogger::info("Disconnected from: %s", k_masterMac);
				reset_state();
			}
			_connected = false;
		}
		// to repeat the action - false to stop
		return true;
	});
}

ESP32_PS4_Controller::~ESP32_PS4_Controller() {
	digitalWrite(k_readyPin, LOW);
	digitalWrite(k_connectedPin, LOW);
	digitalWrite(k_commandReceivedPin, LOW);

	_connected = false;
}

bool ESP32_PS4_Controller::readState() {
	if (PS4.isConnected()) {
		bool stateChanged = checkCommandStates();
		stateChanged |= checkAuxiliaryStates();
		if (stateChanged) {
			toggle(k_commandReceivedPin);
		} else {
			digitalWrite(k_commandReceivedPin, LOW);
		}
		return true;
	} else {
		//SerialLogger::debug("PS4 still not connected to: %s", k_masterMac);
		return false;
	}
}

void ESP32_PS4_Controller::reset_state() {
	m_charging = false;
	m_audioConnected = false;
	m_micConnected = false;
	m_batteryLevel = -1;

	m_lStickX = 0;
	m_lStickY = 0;
	m_rStickX = 0;
	m_rStickY = 0;

	m_downButtonPressed = false;
	m_upButtonPressed = false;
	m_leftButtonPressed = false;
	m_rightButtonPressed = false;

	m_squareButtonPressed = false;
	m_crossButtonPressed = false;
	m_circleButtonPressed = false;
	m_triangleButtonPressed = false;
	m_lbButtonPressed = false;
	m_rbButtonPressed = false;
	m_ltButtonPressed = false;
	m_ltValue = 0;
	m_rtButtonPressed = false;
	m_rtValue = 0;
	m_l3ButtonPressed = false;
	m_r3ButtonPressed = false;
	m_shareButtonPressed = false;
	m_optionsButtonPressed = false;
	m_psButtonPressed = false;
	m_touchpadButtonPressed = false;
}

bool ESP32_PS4_Controller::checkCommandStates() {
	bool stateChanged = false;

	const bool rightButtonPressed = PS4.Right();
	const bool leftButtonPressed = PS4.Left();
	const bool downButtonPressed = PS4.Down();
	const bool upButtonPressed = PS4.Up();

	const int16_t lStickX = PS4.LStickX();
	const int16_t lStickY = PS4.LStickY();
	const int16_t rStickX = PS4.RStickX();
	const int16_t rStickY = PS4.RStickY();

	const bool squareButtonPressed = PS4.Square();
	const bool crossButtonPressed = PS4.Cross();
	const bool circleButtonPressed = PS4.Circle();
	const bool triangleButtonPressed = PS4.Triangle();
	const bool lbButtonPressed = PS4.L1();
	const bool rbButtonPressed = PS4.R1();
	const bool ltButtonPressed = PS4.L2();
	const int16_t ltValue = PS4.L2Value();
	const bool rtButtonPressed = PS4.R2();
	const int16_t rtValue = PS4.R2Value();
	const bool l3ButtonPressed = PS4.L3();
	const bool r3ButtonPressed = PS4.R3();
	const bool shareButtonPressed = PS4.Share();
	const bool optionsButtonPressed = PS4.Options();
	const bool psButtonPressed = PS4.PSButton();
	const bool touchpadButtonPressed = PS4.Touchpad();

	if (rightButtonPressed != m_rightButtonPressed) {
		m_rightButtonPressed = rightButtonPressed;
		SerialLogger::debug("Right Button");
		stateChanged = true;
	}
	if (leftButtonPressed != m_leftButtonPressed) {
		m_leftButtonPressed = leftButtonPressed;
		SerialLogger::debug("Left Button");
		stateChanged = true;
	}
	if (downButtonPressed != m_downButtonPressed) {
		m_downButtonPressed = downButtonPressed;
		SerialLogger::debug("Down Button");
		stateChanged = true;
	}
	if (upButtonPressed != m_upButtonPressed) {
		m_upButtonPressed = upButtonPressed;
		SerialLogger::debug("Up Button");
		stateChanged = true;
	}

	// The axis sticks are highly sensitive. Make less sensitive changes by checking range over exact value
	if (!(m_lStickX - 2 < lStickX && lStickX < m_lStickX + 2)) {
		m_lStickX = lStickX;
		SerialLogger::debug("Left Stick x at %d", m_lStickX);
		stateChanged = true;
	}
	if (!(m_lStickY - 2 < lStickY && lStickY < m_lStickY + 2)) {
		m_lStickY = lStickY;
		SerialLogger::debug("Left Stick y at %d", m_lStickY);
		stateChanged = true;
	}
	if (!(m_rStickX - 2 < rStickX && rStickX < m_rStickX + 2)) {
		m_rStickX = rStickX;
		SerialLogger::debug("Right Stick x at %d", m_rStickX);
		stateChanged = true;
	}
	if (!(m_rStickY - 2 < rStickY && rStickY < m_rStickY + 2)) {
		m_rStickY = rStickY;
		SerialLogger::debug("Right Stick y at %d", m_rStickY);
		stateChanged = true;
	}

	if (squareButtonPressed != m_squareButtonPressed) {
		m_squareButtonPressed = squareButtonPressed;
		SerialLogger::debug("Square Button");
		stateChanged = true;
	}
	if (crossButtonPressed != m_crossButtonPressed) {
		m_crossButtonPressed = crossButtonPressed;
		SerialLogger::debug("Cross Button");
		stateChanged = true;
	}
	if (circleButtonPressed != m_circleButtonPressed) {
		m_circleButtonPressed = circleButtonPressed;
		SerialLogger::debug("Circle Button");
		stateChanged = true;
	}

	if (triangleButtonPressed != m_triangleButtonPressed) {
		m_triangleButtonPressed = triangleButtonPressed;
		SerialLogger::debug("Triangle Button");
		stateChanged = true;
	}
	if (lbButtonPressed != m_lbButtonPressed) {
		m_lbButtonPressed = lbButtonPressed;
		SerialLogger::debug("LB Button");
		stateChanged = true;
	}
	if (rbButtonPressed != m_rbButtonPressed) {
		m_rbButtonPressed = rbButtonPressed;
		SerialLogger::debug("RB Button");
		stateChanged = true;
	}
	if (ltButtonPressed != m_ltButtonPressed || m_ltValue != ltValue) {
		m_ltButtonPressed = ltButtonPressed;
		m_ltValue = ltValue;
		SerialLogger::debug("LT button at %d", m_ltValue);
		stateChanged = true;
	}
	if (rtButtonPressed != m_rtButtonPressed || m_rtValue != rtValue) {
		m_rtButtonPressed = rtButtonPressed;
		m_rtValue = rtValue;
		SerialLogger::debug("RT button at %d", m_rtValue);
		stateChanged = true;
	}
	if (l3ButtonPressed != m_l3ButtonPressed) {
		m_l3ButtonPressed = l3ButtonPressed;
		SerialLogger::debug("L3 Button");
		stateChanged = true;
	}
	if (r3ButtonPressed != m_r3ButtonPressed) {
		m_r3ButtonPressed = r3ButtonPressed;
		SerialLogger::debug("R3 Button");
		stateChanged = true;
	}
	if (shareButtonPressed != m_shareButtonPressed) {
		m_shareButtonPressed = shareButtonPressed;
		SerialLogger::debug("Share Button");
		stateChanged = true;
	}
	if (optionsButtonPressed != m_optionsButtonPressed) {
		m_optionsButtonPressed = optionsButtonPressed;
		SerialLogger::debug("Options Button");
		stateChanged = true;
	}
	if (psButtonPressed != m_psButtonPressed) {
		m_psButtonPressed = psButtonPressed;
		SerialLogger::debug("PS Button");
		stateChanged = true;
	}
	if (touchpadButtonPressed != m_touchpadButtonPressed) {
		m_touchpadButtonPressed = touchpadButtonPressed;
		SerialLogger::debug("Touch Pad Button");
		stateChanged = true;
	}
	return stateChanged;
}

bool ESP32_PS4_Controller::checkAuxiliaryStates() {
	bool stateChanged = false;
	const bool charging = PS4.Charging();
	const bool audioConnected = PS4.Audio();
	const bool micConnected = PS4.Mic();
	const int batteryLevel = PS4.Battery();

	if (m_charging != charging) {
		if (charging) {
			SerialLogger::info("The controller is charging");
		} else {
			SerialLogger::info("The controller is not charging (anymore)");
		}
		m_charging = charging;
		stateChanged = true;
	}
	if (m_audioConnected != audioConnected) {
		if (audioConnected) {
			SerialLogger::info("The controller has headphones attached");
		} else {
			SerialLogger::info("The controller has no headphones attached (anymore)");
		}
		m_audioConnected = audioConnected;
		stateChanged = true;
	}
	if (m_micConnected != micConnected) {
		if (micConnected) {
			SerialLogger::info("The controller has a mic attached");
		} else {
			SerialLogger::info("The controller has no mic attached (anymore)");
		}
		m_micConnected = micConnected;
		stateChanged = true;
	}

	if (m_batteryLevel != batteryLevel) {
		SerialLogger::info("Battery Level : %d", batteryLevel);
		m_batteryLevel = batteryLevel;
		stateChanged = true;
	}
	return stateChanged;
}
