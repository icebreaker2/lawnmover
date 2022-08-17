#ifndef MOTION_STATE_H
#define MOTION_STATE_H

#include <vector>
#include "category.h"

class MotionState {
public:
	MotionState(const char *name, const std::vector <MotionState> &followStates,
				const std::vector <MotionState> &collisionAvoidanceStates,
				const float &left_speed, const float &right_speed, const float blade_speed) :
			k_name(name), k_followStates(followStates), k_collisionAvoidanceStates(collisionAvoidanceStates),
			k_left_speed(left_speed), k_right_speed(right_speed), k_blade_speed(blade_speed) {
		// nothing to do...
	};

	~MotionState() = default;

	/**
	 * Specialised class must determine whether the next state is the followState, errorState or this very state again
	 * @return
	 */
	virtual MotionState getNextState() = 0;

	float get_speed_left() const { return k_left_speed; };

	float get_speed_right() const { return k_right_speed; };

	float get_blade_speed() const { return k_blade_speed; };

	char *get_name() const { return k_name; };

private:
	const char *k_name;
	const std::vector <MotionState> k_followStates;
	const std::vector <MotionState> k_collisionAvoidanceStates;
	const float k_left_speed;
	const float k_right_speed;
	const float k_blade_speed;
};

class ErrorMotion : public MotionState {
public:
	// An error state for states from which we cannot escape to indicate limits of current motion model
	ErrorMotion() : MotionState("ErrorMotion",
								std::vector < MotionState > {},
								std::vector < MotionState > {},
								0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class IdleMotion : public MotionState {
public:
	IdleMotion() : MotionState("IdleMotion",
							   std::vector < MotionState > {},
							   std::vector < MotionState > {},
							   0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class LowSpeedForwardMotion : public MotionState {
public:
	LowSpeedForwardMotion() : MotionState("LowSpeedForwardMotion",
										  std::vector < MotionState > {},
										  std::vector < MotionState > {},
										  CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER, 0.5f) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class MidSpeedForwardMotion : public MotionState {
public:
	MidSpeedForwardMotion() : MotionState("MidSpeedForwardMotion",
										  std::vector < MotionState > {},
										  std::vector < MotionState > {},
										  MID_RANGE_MULTIPLIER, MID_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class FullSpeedForwardMotion : public MotionState {
public:
	FullSpeedForwardMotion() : MotionState("FullSpeedForwardMotion",
										   std::vector < MotionState > {}, std::vector < MotionState > {},
										   OUT_OF_RANGE_MULTIPLIER, OUT_OF_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class CollisionAvoidanceMotion : public MotionState {
public:
	LowSpeedForwardMotion(const char *name, const float &left_speed, const float &right_speed) :
	// TODO idle state or error state
			MotionState(name, std::vector < MotionState > {}, std::vector < MotionState > {}, left_speed, right_speed,
						0.0f) {
		// nothing to do...
	};
};

class BackwardMotion : public CollisionAvoidanceMotion {
public:
	BackwardMotion() : CollisionAvoidanceMotion("BackwardMotion", -CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class LeftTurnMotion : public CollisionAvoidanceMotion {
public:
	LeftTurnMotion() : CollisionAvoidanceMotion("LeftTurnMotion", -CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

class RightTurnMotion : public CollisionAvoidanceMotion {
public:
	LeftTurnMotion() : CollisionAvoidanceMotion("RightTurnMotion", CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	MotionState getNextState() override;
};

#endif // MOTION_STATE_H
