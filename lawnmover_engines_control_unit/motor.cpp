#include "motor.h"
#include <serial_logger.h>

MotorService::MotorService(const int motorPin) :
    kMotorPin(motorPin) {

    pinMode(kMotorPin, OUTPUT);
    stopMotor();
}

MotorService::~MotorService() {
    stopMotor();
}

void MotorService::printInit() {
    SerialLogger::info("Setup MotorService with Pin: %d", kMotorPin);
}

void MotorService::set_rotation_speed(const int16_t rotation_speed) {
    rotation_speed_ = rotation_speed;
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
    if (rotation_speed_ > 10) {
        SerialLogger::debug("Spinning motor with %d/%d", rotation_speed_, 255);
        analogWrite(kMotorPin, rotation_speed_);
    } else {
        SerialLogger::debug("Not spinning motor. Rotation speed was below threshold (%d/10)", rotation_speed_);
        stopMotor();
    }
}
