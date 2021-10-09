#include "ESP32_PS4_Controller.h"
#include <PS4Controller.h>


ESP32_PS4_Controller::ESP32_PS4_Controller(const char *masterMac, const unsigned long timerDelay,
        const int readyPin, const int connectedPin, const int commandReceivedPin, const bool debug)  :
    k_masterMac(String(masterMac)), k_timerDelay(timerDelay), k_readyPin(), k_connectedPin(connectedPin),
    k_commandReceivedPin(commandReceivedPin), k_debug(debug) {

    // LEDs (all on initially)
    //pinMode(k_readyPin, OUTPUT);
    //pinMode(k_connectedPin, OUTPUT);
    //pinMode(k_commandReceivedPin, OUTPUT);
    //digitalWrite(k_readyPin, LOW);
    //digitalWrite(k_connectedPin, LOW);
    //digitalWrite(k_commandReceivedPin, LOW);
    
    Serial.begin(115200);
    Serial.println("Beginning listening for master PS4 Controller connection to: " + k_masterMac);
    PS4.begin(masterMac);

    _internal_timer_handler = [this]() -> bool {
        if (readState()) {
            if (!_connected) {
                Serial.println("Connected to: " + k_masterMac);
                _connected = true;
            }
        } else {
            if (_connected) {
                Serial.println("Disconnected from: " + k_masterMac);
            }
            _connected = false;
        }
        // to repeat the action - false to stop
        return true;
    };
}

void ESP32_PS4_Controller::waitAndRead() {
    if (_internal_timer_handler()) {
        delay(k_timerDelay);
    } else {
        Serial.println("Not waiting before reading PS4 controller again");
    }
}

bool ESP32_PS4_Controller::readState() {
    if (PS4.isConnected()) {
        if (PS4.Right()) Serial.println("Right Button");
        if (PS4.Down()) Serial.println("Down Button");
        if (PS4.Up()) Serial.println("Up Button");
        if (PS4.Left()) Serial.println("Left Button");

        if (PS4.Square()) Serial.println("Square Button");
        if (PS4.Cross()) Serial.println("Cross Button");
        if (PS4.Circle()) Serial.println("Circle Button");
        if (PS4.Triangle()) Serial.println("Triangle Button");

        if (PS4.UpRight()) Serial.println("Up Right");
        if (PS4.DownRight()) Serial.println("Down Right");
        if (PS4.UpLeft()) Serial.println("Up Left");
        if (PS4.DownLeft()) Serial.println("Down Left");

        if (PS4.L1()) Serial.println("L1 Button");
        if (PS4.R1()) Serial.println("R1 Button");

        if (PS4.Share()) Serial.println("Share Button");
        if (PS4.Options()) Serial.println("Options Button");
        if (PS4.L3()) Serial.println("L3 Button");
        if (PS4.R3()) Serial.println("R3 Button");

        if (PS4.PSButton()) Serial.println("PS Button");
        if (PS4.Touchpad()) Serial.println("Touch Pad Button");

        if (PS4.L2()) {
            Serial.printf("L2 button at %d\n", PS4.L2Value());
        }
        if (PS4.R2()) {
            Serial.printf("R2 button at %d\n", PS4.R2Value());
        }

        if (PS4.LStickX()) {
            Serial.printf("Left Stick x at %d\n", PS4.LStickX());
        }
        if (PS4.LStickY()) {
            Serial.printf("Left Stick y at %d\n", PS4.LStickY());
        }
        if (PS4.RStickX()) {
            Serial.printf("Right Stick x at %d\n", PS4.RStickX());
        }
        if (PS4.RStickY()) {
            Serial.printf("Right Stick y at %d\n", PS4.RStickY());
        }

        if (PS4.Charging()) Serial.println("The controller is charging");
        if (PS4.Audio()) Serial.println("The controller has headphones attached");
        if (PS4.Mic()) Serial.println("The controller has a mic attached");

        Serial.printf("Battery Level : %d\n", PS4.Battery());
        Serial.println();
        return true;
    } else {
        return false;
    }
}

ESP32_PS4_Controller::~ESP32_PS4_Controller() {
    //digitalWrite(k_readyPin, LOW);
    //digitalWrite(k_connectedPin, LOW);
    //digitalWrite(k_commandReceivedPin, LOW);

    _connected = false;
}
