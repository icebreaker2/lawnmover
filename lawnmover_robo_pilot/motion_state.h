#ifndef MOTION_STATE_H
#define MOTION_STATE_H

#define DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA (float) 0.70f
#define DEFAULT_WEIGHTED_MOVING_AVERAGE_MIN_MAX_DISCREPANCY (float) 0.30f

#include <vector>
#include <map>
#include <serial_logger.h>

#include "category.h"

class MotionState {
public:
	/**
	 * Init a MotionState chained in other MotionStates and chaining MitoionStates as well
	 *
	 * @param name The name of this motion state
	 * @param max_self_iterations The maximum allowed self iterations of this state (i. e. how may requests on
	 *        getNextState are allowed to this state). Put to -1 to make an infinitive amount.
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
	MotionState(const char *name, const int max_self_iterations, MotionState *followUpState, MotionState *fallbackState,
				const float &left_speed, const float &right_speed, const float blade_speed)
			: k_name(name), k_max_self_iterations(max_self_iterations), k_left_speed(left_speed),
			  k_right_speed(right_speed), k_blade_speed(blade_speed) {
		_followUpState = followUpState;
		_fallbackState = fallbackState;
		_self_iterations = 0;
	};

	~MotionState() = default;

	/**
	 * Specialised class must determine whether the next state is the followUpState, this very state again (if
	 * eligible by check and by max_iterations) or if both of them are not possible the fallbackState
	 * @return
	 */
	virtual MotionState *getNextState(const std::map<Category::Direction, float> &minDistances,
									  const std::map<Category::Direction, float> &maxDistances,
									  const std::map<Category::Direction, float> &weightedMovingAvgDistances) {
		if (k_max_self_iterations < 0) {
			_self_iterations = k_max_self_iterations;
		} else {
			_self_iterations = (_self_iterations + 1) % k_max_self_iterations;
		}
		if (_followUpState == nullptr) {
			if (_self_iterations > 0 && this->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				return this;
			} else {
				_self_iterations = 0;
				return _fallbackState;
			}
		} else if (_followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
			_self_iterations = 0;
			return _followUpState;
		} else {
			if (_self_iterations > 0 && this->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				return this;
			} else {
				_self_iterations = 0;
				return _fallbackState;
			}
		}
	};

	float get_speed_left() const { return k_left_speed; };

	float get_speed_right() const { return k_right_speed; };

	float get_blade_speed() const { return k_blade_speed; };

	const char *get_name() const { return k_name; };

	MotionState *getFollowUpState() const {
		return _followUpState;
	};

	void setFollowUpState(MotionState *motionState) {
		if (_followUpState == nullptr) {
			_followUpState = motionState;
		} else {
			SerialLogger::warn("Cannot reset followUpState. State already occupied by %s", _followUpState->get_name());
		}
	};

	virtual bool isEligible(const std::map<Category::Direction, float> &minDistances,
							const std::map<Category::Direction, float> &maxDistances,
							const std::map<Category::Direction, float> &weightedMovingAvgDistances) const = 0;

protected:
	const int k_max_self_iterations;
	MotionState *_fallbackState;
	MotionState *_followUpState;
	int _self_iterations;

private:
	const char *k_name;

	const float k_left_speed;
	const float k_right_speed;
	const float k_blade_speed;
};

class ErrorMotion : public MotionState {
public:
	// An error state for states from which we cannot escape to indicate limits of current motion model
	ErrorMotion(MotionState *followUpState = nullptr) :
			MotionState("ErrorMotion", -1, followUpState, nullptr, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class IdleMotion : public MotionState {
public:
	IdleMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("IdleMotion", 10, followUpState, fallbackState, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class LowSpeedForwardMotion : public MotionState {
public:
	LowSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("LowSpeedForwardMotion", -1, followUpState, fallbackState,
						CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER, 0.5f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class MidSpeedForwardMotion : public MotionState {
public:
	MidSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("MidSpeedForwardMotion", -1, followUpState, fallbackState,
						MID_RANGE_MULTIPLIER, MID_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class FullSpeedForwardMotion : public MotionState {
public:
	FullSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("FullSpeedForwardMotion", -1, followUpState, fallbackState,
						OUT_OF_RANGE_MULTIPLIER, OUT_OF_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class CollisionAvoidanceMotion : public MotionState {
public:
	/**
	 * Please note: 4 iterations left/right is at the origin position. Child classes need to implement the countering
	 * correctly or otherwise guarantee that motion is done within one iteration.
	 */
	CollisionAvoidanceMotion(const char *name, MotionState *followUpState, MotionState *fallbackState,
							 const float &left_speed, const float &right_speed) :
			MotionState(name, 4, followUpState, fallbackState, left_speed, right_speed, 0.0f) {
		// nothing to do...
	};

};

class LeftTurnMotion : public CollisionAvoidanceMotion {
public:
	LeftTurnMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("LeftTurnMotion", followUpState, fallbackState,
									 -CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class RightTurnMotion : public CollisionAvoidanceMotion {
public:
	RightTurnMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("RightTurnMotion", followUpState, fallbackState,
									 CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class BackwardMotion : public CollisionAvoidanceMotion {
public:
	BackwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("BackwardMotion", followUpState, fallbackState,
									 -CLOSE_RANGE_MULTIPLIER, -CLOSE_RANGE_MULTIPLIER) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};


class PriorityStrategy : public CollisionAvoidanceMotion {
public:
	PriorityStrategy(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("PriorityStrategy", followUpState, fallbackState, 0.0f, 0.0f) {
		// nothing to do...
	};

	/**
	 * Different strategy to get the next state based sorely on the followUpStates decision. This state is just for
	 * convenience.
	 *
	 * @return The next state based on this priority strategy
	 */
	MotionState *getNextState(const std::map<Category::Direction, float> &minDistances,
							  const std::map<Category::Direction, float> &maxDistances,
							  const std::map<Category::Direction, float> &weightedMovingAvgDistances) override {
		if (_followUpState == nullptr && _fallbackState == nullptr) {
			return nullptr;
		} else if (_followUpState == nullptr) {
			return _fallbackState->getNextState(minDistances, maxDistances, weightedMovingAvgDistances);
		} else {
			if (_followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				return _followUpState->getNextState(minDistances, maxDistances, weightedMovingAvgDistances);
			} else {
				return _fallbackState->getNextState(minDistances, maxDistances, weightedMovingAvgDistances);
			}
		}
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override {
		if (_followUpState == nullptr && _fallbackState == nullptr) {
			return false;
		} else if (_followUpState == nullptr) {
			return _fallbackState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances);
		} else {
			if (_followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				return _followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances);
			} else {
				return _fallbackState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances);
			}
		}
	};

private:

};

#endif // MOTION_STATE_H
