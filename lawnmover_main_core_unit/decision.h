#ifndef DECISION_H
#define DECISION_H

#include <Arduino.h>

class MovementDecision {
public:
	MovementDecision(const int16_t left_wheel_power, const int16_t right_wheel_power, const int16_t blade_motor_power) :
			k_left_wheel_power(left_wheel_power), k_right_wheel_power(right_wheel_power),
			k_blade_motor_power(blade_motor_power) {
		// Nothing to do ...
	};

	~MovementDecision() = default;

	int16_t get_left_wheel_power() const {
		return k_left_wheel_power;
	};

	int16_t get_right_wheel_power() const {
		return k_right_wheel_power;
	};

	int16_t get_blade_motor_power() const {
		return k_blade_motor_power;
	};

private:
	const int16_t k_left_wheel_power;
	const int16_t k_right_wheel_power;
	const int16_t k_blade_motor_power;
};

class StopMovementDecision : public MovementDecision {
public:
	StopMovementDecision() : MovementDecision(0, 0, 0) {
		// Nothing to do ...
	};
};

#endif // DECISION_H
