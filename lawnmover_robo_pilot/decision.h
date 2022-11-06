#ifndef DECISION_H
#define DECISION_H

#include "motion_state.h"

#define ENGINE_MAX_POWER_VALUE (int16_t) 255

class MovementDecision {
public:
	static MovementDecision fromState(const MotionState &motionState) {
		return MovementDecision(static_cast<int16_t>(motionState.get_speed_left() * ENGINE_MAX_POWER_VALUE),
		                        static_cast<int16_t>(motionState.get_speed_right() * ENGINE_MAX_POWER_VALUE),
		                        static_cast<int16_t>(motionState.get_blade_speed() * ENGINE_MAX_POWER_VALUE));
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
		return k_left_wheel_power > ENGINE_MAX_POWER_VALUE ? ENGINE_MAX_POWER_VALUE :
		       (k_left_wheel_power < -ENGINE_MAX_POWER_VALUE ? -ENGINE_MAX_POWER_VALUE : k_left_wheel_power);
	};

	int16_t get_right_wheel_power() const {
		return k_right_wheel_power > ENGINE_MAX_POWER_VALUE ? ENGINE_MAX_POWER_VALUE :
		       (k_right_wheel_power < -ENGINE_MAX_POWER_VALUE ? -ENGINE_MAX_POWER_VALUE : k_right_wheel_power);
	};

	int16_t get_blade_motor_power() const {
		return k_blade_motor_power > ENGINE_MAX_POWER_VALUE ? ENGINE_MAX_POWER_VALUE :
		       (k_blade_motor_power < 0 ? 0 : k_blade_motor_power);
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
