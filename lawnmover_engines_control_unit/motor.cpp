#include "motor.h"
#include <serial_logger.h>
#include <spi_commands.h>

MotorService::MotorService(const int motorPin) :
		kMotorPin(motorPin) {

	pinMode(kMotorPin, OUTPUT);
	stopMotor();
    interpretState();
}

MotorService::~MotorService() {
	stopMotor();
}

void MotorService::printInit() {
	SerialLogger::info(F("Setup MotorService with Pin: %d"), kMotorPin);
}

bool MotorService::set_rotation_speed(const int16_t id, const int16_t rotation_speed) {
	// Logging (serial printing is faster) must be kept to an absolute minimum for this SPI command callback depending on the logging baudrate
	// SerialLogger::debug("Inspecting motor speed with id %d and value %d", id, rotation_speed);
	if (id == MOTOR_SPEED_COMMAND) {
        _last_rotation_speed = _rotation_speed;
		_rotation_speed = rotation_speed;
		return true;
	} else {
		return false;
	}
}

void MotorService::startMotor() {
	SerialLogger::trace(F("Starting motor"));
    set_rotation_speed(MOTOR_SPEED_COMMAND, 255);
}

void MotorService::stopMotor() {
	SerialLogger::trace(F("Stopping motor"));
    set_rotation_speed(MOTOR_SPEED_COMMAND, 0);
}

void MotorService::interpretState() {
	if (_rotation_speed > 10) {
        if (_rotation_speed != _last_rotation_speed) {
            SerialLogger::debug(F("Spinning motor with %d/%d"), _rotation_speed, 255);
            analogWrite(kMotorPin, _rotation_speed);
        }
	} else {
        SerialLogger::trace(F("Rotation speed was below threshold (%d/10). Stop motor spinning."), _rotation_speed);
		stopMotor();
        analogWrite(kMotorPin, _rotation_speed);
	}
}
