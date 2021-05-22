#include "IrCtrl.h"

IRCtrl::IRCtrl(const int irReceivePin, MoverService &moverService, MotorService &motorService) :
    kIrReceivePin(irReceivePin) {
    _moverService = moverService;
    _motorService = motorService;

    // IR remote commands
    IrReceiver.begin(kIrReceivePin, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
}

IRCtrl::~IRCtrl() {
    // nothing to do ...
}

boolean IRCtrl::rawSpinCheck() {
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

void IRCtrl::checkAndApplyIRCommand() {
    if (IrReceiver.decode()) {
        boolean repeat = IrReceiver.decodedIRData.decodedRawData == 0;
        if (repeat) {
            if (IrReceiver.decodedIRData.command == 0xD) {
                // motor on / play and pause
                _motorService.spinMotor();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // motor stop / play and pause
                _motorService.stopMotor();
            } else {
                //Serial.println("Unknown repeat command");
                //IrReceiver.printIRResultShort(&Serial);
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
                // Func / stop (Movement)
                Serial.println(" Func / stop");
                _moverService.stopMovement();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // motor stop / play and pause
                _motorService. stopMotor();
            } else if (IrReceiver.decodedIRData.command == 0xD) {
                _motorService.startMotor();
            } else {
                //Serial.println("Unknown single command");
                //IrReceiver.printIRResultShort(&Serial);
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    // Print a short summary of received data
                    Serial.print("0: ");
                    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[0]);
                    Serial.print(", 1: ");
                    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[1]);
                    Serial.print(", 2: ");
                    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[2]);
                    Serial.print(", 3: ");
                    Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawbuf[3]);
                    // We have an unknown protocol here, print more info
                    IrReceiver.printIRResultRawFormatted(&Serial, true);
                }
            }
        }
        /*
            !!!Important!!! Enable receiving of the next value,
            since receiving has stopped after the end of the current received data packet.
        */
        IrReceiver.resume(); // Enable receiving of the next value
    }
}
