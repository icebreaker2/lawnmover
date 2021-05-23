#include "Motor.h"

// TODO This is bad. Fox the timer lib to take this or at least make singleton of this service if timer.every still wont work passing this or a callback / lambda into it
MotorService *uniqueMotorService;

MotorService::MotorService(const int motorPin, Timer<> &timer) :
    kMotorPin(motorPin) {
    stopMotor();

    uniqueMotorService = this;

    timer.every(MOTOR_SPIN_CHECK_TIME_DELAY, [](void*) -> bool {
        //        checkAndResetMotorCmd();
        uniqueMotorService->checkAndResetMotorCmd();
        return true; // to repeat the action - false to stop
    });

    Serial.print("Setup MotorService with Pin: ");
    Serial.println(kMotorPin);
}

MotorService::~MotorService() {
    stopMotor();
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
