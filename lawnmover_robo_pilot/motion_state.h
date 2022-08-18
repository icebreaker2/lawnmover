#ifndef MOTION_STATE_H
#define MOTION_STATE_H

#include <serial_logger.h>

#include "category.h"

class MotionState {
public:
	/**
	 * Init a MotionState chained in other MotionStates and chaining MitoionStates as well
	 *
	 * @param name The name of this motion state
	 * @param max_self_iterations The maximum allowed self iterations of this state (i. e. how may requests on
	 *        getNextStates are allowed to this state). Put to -1 to make an infinitive amount.
	 * @param followUpState The next (better) following state to enter if eligible or nullptr if None exists. Caller
	 *        does not put ownership of this pointer in this state but needs to guarantee it lives as long as this state.
	 *        We chain states, thus, its likely that we produce a cycle. A cycle would case segsev upon deletion.
	 * @param fallbackState The (previous) state to enter if neither the followUp nor this state are eligible (anymore)
	 *        or nullptr if None exists. Caller does not put ownership of this pointer in this state but needs to
	 *        guarantee it lives as long as this state. We chain states, thus, its likely that we produce a cycle.
	 *        A cycle would case segsev upon deletion.
	 * @param left_speed The speed to apply to the left wheel in percentage of power. Negative for backwards.
	 * @param right_speed The speed to apply to the right wheel in percentage of power. Negative for backwards.
	 * @param blade_speed The speed to apply to the blade motor in percentage of power
	 */
	MotionState(const char *name, const int max_self_iterations, const MotionState *followUpState,
				const MotionState *fallbackState, const float &left_speed, const float &right_speed,
				const float blade_speed) :
			k_name(name), k_max_self_iterations(max_self_iterations), _followUpState(followUpState),
			k_fallbackState(fallbackState), k_left_speed(left_speed), k_right_speed(right_speed),
			k_blade_speed(blade_speed) {
		// nothing to do...
	};

	~MotionState() = default;

	/**
	 * Specialised class must determine whether the next state is the followUpState, this very state again (if
	 * eligible by max_iterations) or if both of them are not possible the fallbackState
	 * @return
	 */
	const MotionState *getNextState() const {
		// TODO implement as described
	};

	float get_speed_left() const { return k_left_speed; };

	float get_speed_right() const { return k_right_speed; };

	float get_blade_speed() const { return k_blade_speed; };

	const char *get_name() const { return k_name; };

	const MotionState *getFollowUpState() const {
		return _followUpState;
	};

	void setFollowUpState(const MotionState *motionState) {
		if (_followUpState == nullptr) {
			_followUpState = motionState;
		} else {
			SerialLogger::warn("Cannot reset followUpState. State already occupied by %s", _followUpState->get_name());
		}
	};

protected:
	// TODO add distances (front, front_left, front_right, back_left, back_right) and heading just received for getNexttState and isEligble
	virtual bool isEligible() const = 0;

private:
	const char *k_name;

	const int k_max_self_iterations;
	const MotionState *_followUpState;
	const MotionState *k_fallbackState;

	const float k_left_speed;
	const float k_right_speed;
	const float k_blade_speed;
};

class ErrorMotion : public MotionState {
public:
	// An error state for states from which we cannot escape to indicate limits of current motion model
	ErrorMotion(const MotionState *followUpState = nullptr) :
			MotionState("ErrorMotion", -1, followUpState, nullptr, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class IdleMotion : public MotionState {
public:
	IdleMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			MotionState("IdleMotion", 10, followUpState, fallbackState, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class LowSpeedForwardMotion : public MotionState {
public:
	LowSpeedForwardMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			MotionState("LowSpeedForwardMotion", -1, followUpState, fallbackState,
						CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER, 0.5f) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class MidSpeedForwardMotion : public MotionState {
public:
	MidSpeedForwardMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			MotionState("MidSpeedForwardMotion", -1, followUpState, fallbackState,
						MID_RANGE_MULTIPLIER, MID_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class FullSpeedForwardMotion : public MotionState {
public:
	FullSpeedForwardMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			MotionState("FullSpeedForwardMotion", -1, followUpState, fallbackState,
						OUT_OF_RANGE_MULTIPLIER, OUT_OF_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class CollisionAvoidanceMotion : public MotionState {
public:
	/**
	 * Please note: 4 iterations left/right is at the origin position. Child classes need to implement the countering
	 * correctly or otherwise guarantee that motion is done within one iteration.
	 */
	CollisionAvoidanceMotion(const char *name, const MotionState *followUpState, const MotionState *fallbackState,
							 const float &left_speed, const float &right_speed) :
			MotionState(name, 4, followUpState, fallbackState, left_speed, right_speed, 0.0f) {
		// nothing to do...
	};
};

class LeftTurnMotion : public CollisionAvoidanceMotion {
public:
	LeftTurnMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			CollisionAvoidanceMotion("LeftTurnMotion", followUpState, fallbackState,
									 -CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class RightTurnMotion : public CollisionAvoidanceMotion {
public:
	RightTurnMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			CollisionAvoidanceMotion("RightTurnMotion", followUpState, fallbackState,
									 CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible() const override;
};

class BackwardMotion : public CollisionAvoidanceMotion {
public:
	BackwardMotion(const MotionState *followUpState, const MotionState *fallbackState) :
			CollisionAvoidanceMotion("BackwardMotion", followUpState, fallbackState,
									 -CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible() const override;
};

#endif // MOTION_STATE_H
