#include <arduino_timer_uno.h>
#include <IRremote.h>

#include "Mover.h"
#include "Motor.h"
#include "LED.h"

auto _timer = timer_create_default();

/**
    Note: I had major issues using pin 13 while using IR. Pin 13 was always high. Maybe my arduino
    is not feeling well or simple wants to annoy me but, I changed the left pins from 13 and 12
    to 5 and 4 -- fixed the issue. Did not traced the issue any further for reasons of time.
*/
const int DANGEROUS_PINS_WITH_IR_LIB = 13;

const int LEFT_FWD_PIN = 9; // is PWM
const int LEFT_BWD_PIN = 8; // is no PWM
const int LEFT_PWM_PIN = 3; // is PWM
const int RIGHT_PWM_PIN = 10; // is PWM
const int RIGHT_FWD_PIN = 5; // is PWM
const int RIGHT_BWD_PIN = 4; // is no PWM

const int LEFT_FWD_PWM = 255; // maximum
const int LEFT_BWD_PWM = 255; // maximum
const int RIGHT_FWD_PWM = 255; // maximum
const int RIGHT_BWD_PWM = 255; // maximum

MoverService _moverService(LEFT_FWD_PIN, LEFT_BWD_PIN, LEFT_PWM_PIN, RIGHT_PWM_PIN, RIGHT_FWD_PIN,
                           RIGHT_BWD_PIN, LEFT_FWD_PWM, LEFT_BWD_PWM, RIGHT_FWD_PWM, RIGHT_BWD_PWM, true);

const int MOTOR_PIN = 2; // no PWM / control with 5v only
MotorService _motorService(MOTOR_PIN, _timer, false);

const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

const int IR_COMMAND_RECV = 6;

// Debug
//const int DEBUG_PIN_1 = 2;
//const int DEBUG_PIN_2 = 3;
//const int DEBUG_PIN_3 = 4;

boolean rawSpinCheck() {
    if (IrReceiver.decodedIRData.rawDataPtr->rawlen == 4) {
        const int firstByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[0];
        const int secondByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[1];
        const int thirdByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[2];
        const int fourthByte =  IrReceiver.decodedIRData.rawDataPtr->rawbuf[3];
        if (firstByte >= 1870 && firstByte <= 2050 && secondByte >= 177 && secondByte <= 182 && thirdByte >= 43 && thirdByte <= 47 && fourthByte >= 10 && fourthByte <= 13) {
            // observed in various tests
            _motorService.spinMotor();
            return true;
        }
    }
    return false;
}

void checkAndApplyIRCommand() {
    if (IrReceiver.decode()) {
        boolean repeat = IrReceiver.decodedIRData.decodedRawData == 0;
        if (repeat) {
            if (IrReceiver.decodedIRData.command == 0xD) {
                // motor on / play and pause
                _motorService.spinMotor();
            } else if (IrReceiver.decodedIRData.command == 0x47) {
                // Func / stop (motor)
                _motorService.stopMotor();
            } else if (IrReceiver.decodedIRData.command == 0x44) {
                // left / previous
                _moverService.turnLeft();
            } else if (IrReceiver.decodedIRData.command == 0x43) {
                // right / next
                _moverService.turnRight();
            } else if (IrReceiver.decodedIRData.command == 0x9) {
                // forward / up
                _moverService.moveForward();
            } else if (IrReceiver.decodedIRData.command == 0x7) {
                // backwards / down
                _moverService.moveBackward();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // movement stop / play and pause
                Serial.println("play / pause");
                _moverService.stopMovement();
            } else {
                Serial.println("Unknown repeat command");
                IrReceiver.printIRResultShort(&Serial);
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    // We have an unknown protocol here, print more info
                    if (!rawSpinCheck()) {
                        Serial.print("0: ");
                        Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[0]);
                        Serial.print(", 1: ");
                        Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[1]);
                        Serial.print(", 2: ");
                        Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[2]);
                        Serial.print(", 3: ");
                        Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawbuf[3]);
                        IrReceiver.printIRResultRawFormatted(&Serial, true);
                    }
                }
            }
        } else {
            // Finally, check the received data and perform actions according to the received command/
            if (IrReceiver.decodedIRData.command == 0x44) {
                // left / previous
                _moverService.turnLeft();
            } else if (IrReceiver.decodedIRData.command == 0x43) {
                // right / next
                _moverService.turnRight();
            } else if (IrReceiver.decodedIRData.command == 0x9) {
                // forward / up
                _moverService.moveForward();
            } else if (IrReceiver.decodedIRData.command == 0x7) {
                // backwards / down
                _moverService.moveBackward();
            } else if (IrReceiver.decodedIRData.command == 0x47) {
                // Func / stop (motor)
                _motorService.stopMotor();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // movement stop / play and pause
                Serial.println("play / pause");
                _moverService.stopMovement();
            } else if (IrReceiver.decodedIRData.command == 0xD) {
                _motorService.startMotor();
            } else {
                //Serial.println("Unknown single command");
                //IrReceiver.printIRResultShort(&Serial);
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    // Print a short summary of received data
                    // Serial.print("0: ");
                    // Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[0]);
                    // Serial.print(", 1: ");
                    // Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[1]);
                    // Serial.print(", 2: ");
                    // Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[2]);
                    // Serial.print(", 3: ");
                    // Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawbuf[3]);
                    // We have an unknown protocol here, print more info
                    // IrReceiver.printIRResultRawFormatted(&Serial, true);
                }
            }
        }
        IrReceiver.resume(); // Enable receiving of the next value
    } else {
        //        Serial.println("(Debug) No signal received");
    }
}



void setup() {
    // sr debug
    Serial.begin(115200);
    _moverService.printInit();
    _motorService.printInit();
    Led3Service _ledService(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);

    // IR remote commands
    IrReceiver.begin(IR_COMMAND_RECV, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);

    // debug pin always high
    //    pinMode(DEBUG_PIN_1, OUTPUT);
    //    digitalWrite(DEBUG_PIN_1, HIGH);
    //    pinMode(DEBUG_PIN_2, OUTPUT);
    //    digitalWrite(DEBUG_PIN_2, HIGH);
    //    pinMode(DEBUG_PIN_3, OUTPUT);
    //    digitalWrite(DEBUG_PIN_3, HIGH);

    _motorService.stopMotor();
    _moverService.stopMovement();

    // TODO enable dead mens button check again

    // DEBUG START
    //    _moverService.moveForward();
    //    _moverService.moveBackward();
    //    _moverService.turnRight();
    //    _moverService.turnLeft();

//    _motorService.startMotor();
    // DEBUG END
}

void loop() {
    // DEBUG START
    //        _moverService.changeLeftPwmRate(255);
    //        _moverService.changeRightPwmRate(255);
    //        Serial.println("change to 255");
    //        delay(1500);
    //
    //        _moverService.changeLeftPwmRate(128);
    //        _moverService.changeRightPwmRate(128);
    //        Serial.println("change to 128");
    //        delay(1500);
    //
    //        _moverService.changeLeftPwmRate(56);
    //        _moverService.changeRightPwmRate(56);
    //        Serial.println("change to 56");
    //        delay(1500);
    //
    //        _moverService.changeLeftPwmRate(28);
    //        _moverService.changeRightPwmRate(28);
    //        Serial.println("change to 28");
    //        delay(2500);
    //
    //        _moverService.changeLeftPwmRate(0);
    //        _moverService.changeRightPwmRate(0);
    //        Serial.println("change to 0");
    //        delay(1000);

    _moverService.moveForward();
    delay(3000);
    _moverService.turnRight();
    delay(3000);
    _moverService.turnLeft();
    delay(3000);
    _moverService.moveBackward();
    delay(3000);
    // DEBUG END

    checkAndApplyIRCommand();

    //    Serial.println("(Debug) Turn");
    // tick timers
    auto ticks = _timer.tick();
}