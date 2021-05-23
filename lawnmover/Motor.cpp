#include "Motor.h"

// TODO This is bad. Fox the timer lib to take this or at least make singleton of this service if timer.every still wont work passing this or a callback / lambda into it
MotorService *uniqueMotorService;
bool _deadMansSwitch;

MotorService::MotorService(const int motorPin, Timer<> &timer, const bool deadMansSwitch) :
    kMotorPin(motorPin) {
    stopMotor();

    uniqueMotorService = this;
    _deadMansSwitch = deadMansSwitch;
    timer.every(MOTOR_SPIN_CHECK_TIME_DELAY, [](void*) -> bool {
        //        checkAndResetMotorCmd();
        if (_deadMansSwitch) {
            uniqueMotorService->checkAndResetMotorCmd();
        }
        return true; // to repeat the action - false to stop
    });

    printInit();
}

MotorService::~MotorService() {
    stopMotor();
}

void MotorService::printInit() {
    Serial.print("Setup MotorService with Pin: ");
    Serial.println(kMotorPin);
}

void MotorService::startMotor() {
    _motorSpinCmdReceived++;
    Serial.println("start motor (ST/REPT)");
    digitalWrite(kMotorPin, HIGH);
}

void MotorService::stopMotor() {
    Serial.println("motor stop / pause");
    digitalWrite(kMotorPin, LOW);
}

void MotorService::spinMotor() {
    _motorSpinCmdReceived++;
    Serial.println("motor spin / ST/REPT");
    digitalWrite(kMotorPin, HIGH);
}

void MotorService::checkAndResetMotorCmd() {
    Serial.print("MotorCtr after ");
    Serial.print(MOTOR_SPIN_CHECK_TIME_DELAY);
    Serial.print(" ms was: ");
    Serial.print(_motorSpinCmdReceived);
    Serial.print("/");
    Serial.println(MOTOR_SPIN_CHECK_THRESHOLD);
    if (_motorSpinCmdReceived < MOTOR_SPIN_CHECK_THRESHOLD) {
        stopMotor();
    }
    _motorSpinCmdReceived = 0; // reset
}
