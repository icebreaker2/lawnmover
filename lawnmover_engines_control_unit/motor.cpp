#include "motor.h"
#include <serial_logger.h>

MotorService::MotorService(const int motorPin, const int motorSpeedCommandId) :
    kMotorPin(motorPin), k_motorSpeedCommandId(motorSpeedCommandId) {

    pinMode(kMotorPin, OUTPUT);
    stopMotor();
}

MotorService::~MotorService() {
    stopMotor();
}

void MotorService::printInit() {
    SerialLogger::info("Setup MotorService with Pin: %d", kMotorPin);
}

bool MotorService::set_rotation_speed(const int16_t id, const int16_t rotation_speed) {
    if (id == k_motorSpeedCommandId) {
        _rotation_speed = rotation_speed;
        return true;
    } else {
        return false;
    }
}

void MotorService::startMotor() {
    SerialLogger::trace("Starting motor");
    analogWrite(kMotorPin, 128);
}

void MotorService::stopMotor() {
    SerialLogger::trace("Stopping motor");
    analogWrite(kMotorPin, 0);
}

void MotorService::spinMotor() {
    if (_rotation_speed > 10) {
        SerialLogger::debug("Spinning motor with %d/%d", _rotation_speed, 255);
        analogWrite(kMotorPin, _rotation_speed);
    } else {
        SerialLogger::debug("Rotation speed was below threshold (%d/10). Stop motor spinning.", _rotation_speed);
        stopMotor();
    }
}
