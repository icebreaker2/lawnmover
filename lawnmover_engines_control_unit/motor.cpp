#include "motor.h"
#include <serial_logger.h>
#include <spi_commands.h>

MotorService::MotorService(const int motorPin) :
		kMotorPin(motorPin) {

	pinMode(kMotorPin, OUTPUT);
	stopMotor();
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
		_rotation_speed = rotation_speed;
		return true;
	} else {
		return false;
	}
}

void MotorService::startMotor() {
	SerialLogger::trace(F("Starting motor"));
	analogWrite(kMotorPin, 255);
}

void MotorService::stopMotor() {
	SerialLogger::trace(F("Stopping motor"));
	analogWrite(kMotorPin, 0);
}

void MotorService::spinMotor() {
	if (_rotation_speed > 10) {
		SerialLogger::debug(F("Spinning motor with %d/%d"), _rotation_speed, 255);
		analogWrite(kMotorPin, _rotation_speed);
	} else {
		SerialLogger::debug(F("Rotation speed was below threshold (%d/10). Stop motor spinning."), _rotation_speed);
		stopMotor();
	}
}
