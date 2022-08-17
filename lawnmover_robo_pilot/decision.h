#ifndef DECISION_H
#define DECISION_H

#include "motion_state.h"

class MovementDecision {
public:
	static MovementDecision fromState(const MotionState &motionState) {
		return MovementDecision(static_cast<int16_t>(motionState.get_speed_left() * k_max_power_value),
								static_cast<int16_t>(motionState.get_speed_right() * k_max_power_value),
								static_cast<int16_t>(motionState.get_blade_speed() * k_max_power_value));
	};

	/**
	 * Create a new movement decision
	 *
	 * @param left_wheel_power left wheel power values need to be in range of [-255,255]
	 * @param right_wheel_power right wheel power values need to be in range of [-255,255]
	 * @param blade_motor_power blade motor power values need to be in range of [0,255]
	 */
	MovementDecision(const int16_t left_wheel_power, const int16_t right_wheel_power, const int16_t blade_motor_power) :
			k_left_wheel_power(left_wheel_power), k_right_wheel_power(right_wheel_power),
			k_blade_motor_power(blade_motor_power) {
		// Nothing to do ...
	};

	~MovementDecision() = default;

	int16_t get_left_wheel_power() const {
		return k_left_wheel_power > k_max_power_value ? k_max_power_value :
			   (k_left_wheel_power < -k_max_power_value ? -k_max_power_value : k_left_wheel_power);
	};

	int16_t get_right_wheel_power() const {
		return k_right_wheel_power > k_max_power_value ? k_max_power_value :
			   (k_right_wheel_power < -k_max_power_value ? -k_max_power_value : k_right_wheel_power);
	};

	int16_t get_blade_motor_power() const {
		return k_blade_motor_power > k_max_power_value ? k_max_power_value :
			   (k_blade_motor_power < 0 : 0 : k_blade_motor_power);
	};

private:
	const int16_t k_max_power_value = 255;

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
