#include "ESP32_PS4_Controller.h"
#include <PS4Controller.h>
#include <SerialLogger.h>

void toggle(const int pin) {
    if (digitalRead(pin) == 0) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }
}

ESP32_PS4_Controller::ESP32_PS4_Controller(Timer<> &timer, const char *masterMac, const unsigned long timerDelay,
        const int readyPin, const int connectedPin, const int commandReceivedPin, const bool debug)  :
    k_masterMac(masterMac), k_timerDelay(timerDelay), k_readyPin(), k_connectedPin(connectedPin),
    k_commandReceivedPin(commandReceivedPin), k_debug(debug) {

    // LEDs (all on initially)
    pinMode(k_readyPin, OUTPUT);
    pinMode(k_connectedPin, OUTPUT);
    pinMode(k_commandReceivedPin, OUTPUT);
    digitalWrite(k_readyPin, LOW);
    digitalWrite(k_connectedPin, LOW);
    digitalWrite(k_commandReceivedPin, LOW);

    SerialLogger::info("Beginning listening for master PS4 Controller connection to: %s", k_masterMac);
    PS4.begin(masterMac);

    timer.every(k_timerDelay, [this](void*) -> bool {
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
        return false;
    }
}

bool  ESP32_PS4_Controller::checkCommandStates() {
    bool stateChanged = false;
    if (PS4.Right()) {
        SerialLogger::debug("Right Button");
        stateChanged = true;
    }
    if (PS4.Down()) {
        SerialLogger::debug("Down Button");
        stateChanged = true;
    }
    if (PS4.Up()) {
        SerialLogger::debug("Up Button");
        stateChanged = true;
    }
    if (PS4.Left()) {
        SerialLogger::debug("Left Button");
        stateChanged = true;
    }

    const int lStickX = PS4.LStickX();
    const int lStickY = PS4.LStickY();
    const int rStickX = PS4.RStickX();
    const int rStickY = PS4.RStickY();

    // The axis sticks are highly sensitive. Make less sensitive changes by checking range over exact value
    if (!(m_lStickX - 2 < lStickX  && lStickX  < m_lStickX + 2)) {
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

    if (PS4.Square()) {
        SerialLogger::debug("Square Button");
        stateChanged = true;
    }
    if (PS4.Cross()) {
        SerialLogger::debug("Cross Button");
        stateChanged = true;
    }
    if (PS4.Circle()) {
        SerialLogger::debug("Circle Button");
        stateChanged = true;
    }
    if (PS4.Triangle()) {
        SerialLogger::debug("Triangle Button");
        stateChanged = true;
    }

    if (PS4.L1()) {
        SerialLogger::debug("LB Button");
        stateChanged = true;
    }
    if (PS4.R1()) {
        SerialLogger::debug("RB Button");
        stateChanged = true;
    }

    if (PS4.L2()) {
        SerialLogger::debug("LT button at %d", PS4.L2Value());
        stateChanged = true;
    }
    if (PS4.R2()) {
        SerialLogger::debug("RT button at %d", PS4.R2Value());
        stateChanged = true;
    }

    if (PS4.L3()) {
        SerialLogger::debug("L3 Button");
        stateChanged = true;
    }
    if (PS4.R3()) {
        SerialLogger::debug("R3 Button");
        stateChanged = true;
    }

    if (PS4.Share()) {
        SerialLogger::debug("Share Button");
        stateChanged = true;
    }
    if (PS4.Options()) {
        SerialLogger::debug("Options Button");
        stateChanged = true;
    }

    if (PS4.PSButton()) {
        SerialLogger::debug("PS Button");
        stateChanged = true;
    }
    if (PS4.Touchpad()) {
        SerialLogger::debug("Touch Pad Button");
        stateChanged = true;
    }
    return stateChanged;
}

bool ESP32_PS4_Controller::checkAuxiliaryStates() {
    bool stateChanged = false;
    const bool isCharging = PS4.Charging();
    const bool isAudioConnected = PS4.Audio();
    const bool isMicConnected = PS4.Mic();
    const int batteryLevel = PS4.Battery();

    if (m_isCharging != isCharging) {
        if (isCharging) {
            SerialLogger::info("The controller is charging");
        } else {
            SerialLogger::info("The controller is not charging (anymore)");
        }
        m_isCharging = isCharging;
        stateChanged = true;
    }
    if (m_isAudioConnected != isAudioConnected) {
        if (isAudioConnected) {
            SerialLogger::info("The controller has headphones attached");
        } else {
            SerialLogger::info("The controller has no headphones attached (anymore)");
        }
        m_isAudioConnected = isAudioConnected;
        stateChanged = true;
    }
    if (m_isMicConnected != isMicConnected) {
        if (isMicConnected) {
            SerialLogger::info("The controller has a mic attached");
        } else {
            SerialLogger::info("The controller has no mic attached (anymore)");
        }
        m_isMicConnected = isMicConnected;
        stateChanged = true;
    }

    if (m_batteryLevel != batteryLevel) {
        SerialLogger::info("Battery Level : %d", batteryLevel);
        m_batteryLevel = batteryLevel;
        stateChanged = true;
    }
    return stateChanged;
}
