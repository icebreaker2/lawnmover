#include "motor.h"
#include <serial_logger.h>

// TODO This is bad. The timer accept C-like function pointer only. I replaced it with a version using std::function for esp32 but this will not compile due to missing function headers in classic arduino compiler.
MotorService *uniqueMotorService;
bool _deadMansSwitch;

MotorService::MotorService(const int motorPin, Timer<> &timer, const bool deadMansSwitch) :
    kMotorPin(motorPin) {

    pinMode(kMotorPin, OUTPUT);
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
}

MotorService::~MotorService() {
    stopMotor();
}

void MotorService::printInit() {
    SerialLogger::info("Setup MotorService with Pin: %d", kMotorPin);
}

void MotorService::startMotor() {
    _motorSpinCmdReceived++;
    SerialLogger::info("start motor (ST/REPT)");
    digitalWrite(kMotorPin, HIGH);
}

void MotorService::stopMotor() {
    SerialLogger::info("motor Func / stop");
    digitalWrite(kMotorPin, LOW);
}

void MotorService::spinMotor() {
    _motorSpinCmdReceived++;
    SerialLogger::debug("motor spin / ST/REPT");
    digitalWrite(kMotorPin, HIGH);
}

void MotorService::checkAndResetMotorCmd() {
    SerialLogger::debug("MotorCtr after %d %s %d/%d", MOTOR_SPIN_CHECK_TIME_DELAY, "ms was:", _motorSpinCmdReceived, MOTOR_SPIN_CHECK_THRESHOLD);
    if (_motorSpinCmdReceived < MOTOR_SPIN_CHECK_THRESHOLD) {
        stopMotor();
    }
    _motorSpinCmdReceived = 0; // reset
}
