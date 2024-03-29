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
	 * @param min_self_iterations The minimum amount of self iterations this states needs to perform (i. e. how may
 	 * 		  requests on getNextState returning this) before the follow up states is considered an option.
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
	MotionState(const char *name, const int min_self_iterations, const int max_self_iterations,
				MotionState *followUpState, MotionState *fallbackState,
				const float &left_speed, const float &right_speed, const float blade_speed)
			: k_name(name), k_min_self_iterations(min_self_iterations), k_max_self_iterations(max_self_iterations),
			  k_left_speed(left_speed), k_right_speed(right_speed), k_blade_speed(blade_speed) {
		_followUpState = followUpState;
		_fallbackState = fallbackState;
		if (fallbackState == nullptr) {
			SerialLogger::error(F("nullptr fallback states are not allowed. Expect serious issues"));
		}
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
		_self_iterations++;
		if (_followUpState == nullptr) {
			if ((k_max_self_iterations < 0 || _self_iterations <= k_max_self_iterations) &&
				this->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				SerialLogger::trace(F("No follow up state and %s is still eligible, thus, returning it"),
									this->get_name());
				return this;
			} else {
				SerialLogger::trace(F("%s no longer eligible, trying fallback due to nullptr followUpState and "
									  "non-eligibility"), this->get_name());
				return changeState(_fallbackState, minDistances, maxDistances, weightedMovingAvgDistances);
			}
		} else if (delayedFollowUp(minDistances, maxDistances, weightedMovingAvgDistances)) {
			// Note: We do not check the self iterations for good reasons. Assume we stop a left turn at a random step
			// t=3 while having reached an angle of 30/90°. If we now move to the fallback, we start with a wrong angle.
			// If we already use another function to determine eligibility, we need and can safely drop the check.
			if (this->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				SerialLogger::trace(F("Returning %s due to delayed follow up and eligibility"), this->get_name());
				return this;
			} else {
				SerialLogger::trace(F("%s no longer eligible, trying fallback due to delayed follow up but "
									  "non-eligibility"), this->get_name());
				return changeState(_fallbackState, minDistances, maxDistances, weightedMovingAvgDistances);
			}
		} else if (_followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
			SerialLogger::debug(F("Returning follow up state due to non-delayed follow up and follow-up states "
								  "eligibility"));
			return changeState(_followUpState, minDistances, maxDistances, weightedMovingAvgDistances);
		} else {
			if ((k_max_self_iterations < 0 || _self_iterations <= k_max_self_iterations) &&
				this->isEligible(minDistances, maxDistances, weightedMovingAvgDistances)) {
				SerialLogger::trace(F("Follow up state is not eligible but this %s is still eligible, thus, returning "
									  "it"), this->get_name());
				return this;
			} else {
				SerialLogger::trace(F("Trying fallback due to non-delayed follow up and states non-eligibility"));
				return changeState(_fallbackState, minDistances, maxDistances, weightedMovingAvgDistances);
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
			SerialLogger::warn(F("Cannot reset followUpState. State already occupied by %s"),
							   _followUpState->get_name());
		}
	};

	/**
	 * Special treatment for some MotionStates which do not want to try to immediately move the the next follow up state
	 * (e. g. if IdleState is always possible and we are in a CollisionAvoidanceState)
	 *
	 * @param minDistances The min distances of the history (configurable) for each direction.
	 * @param maxDistances The max distances of the history (configurable) for each direction.
	 * @param weightedMovingAvgDistances The weighted moving averages of distances (configurable) for each direction.
	 * @return Whether this state must be executed again or not by extra options.
	 */
	virtual bool delayedFollowUp(const std::map<Category::Direction, float> &minDistances,
								 const std::map<Category::Direction, float> &maxDistances,
								 const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
		return _self_iterations <= k_min_self_iterations;
	};

	virtual bool isEligible(const std::map<Category::Direction, float> &minDistances,
							const std::map<Category::Direction, float> &maxDistances,
							const std::map<Category::Direction, float> &weightedMovingAvgDistances) const = 0;

protected:
	const int k_min_self_iterations;
	const int k_max_self_iterations;
	MotionState *_fallbackState;
	MotionState *_followUpState;
	int _self_iterations;

private:
	MotionState *changeState(MotionState *nextState, const std::map<Category::Direction, float> &minDistances,
							 const std::map<Category::Direction, float> &maxDistances,
							 const std::map<Category::Direction, float> &weightedMovingAvgDistances) {
		_self_iterations = 0;
		if (nextState == nullptr) {
			return nextState;
		} else {
			SerialLogger::debug(F("Trying next state %s"), nextState->get_name());
			return nextState->getNextState(minDistances, maxDistances, weightedMovingAvgDistances);
		}
	};

	const char *k_name;

	const float k_left_speed;
	const float k_right_speed;
	const float k_blade_speed;
};

class ErrorMotion : public MotionState {
public:
	// An error state for states from which we cannot escape to indicate limits of current motion model
	ErrorMotion(MotionState *followUpState = nullptr) :
			MotionState("ErrorMotion", 1, -1, followUpState, nullptr, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class IdleMotion : public MotionState {
public:
	IdleMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("IdleMotion", 1, 1, followUpState, fallbackState, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class ForwardMotion : public MotionState {
public:
	ForwardMotion(const char *name, MotionState *followUpState, MotionState *fallbackState,
				  const float &left_speed, const float &right_speed, const float blade_speed) :
			MotionState(name, 3, -1, followUpState, fallbackState, left_speed, right_speed, blade_speed) {
		// nothing to do...
	};

	// TODO once added angle/azitmuth we need to take this value into account for the left, right speed to keep on the
	//  line. Therefore, we need to be able to modify the return value (maybe overwrite the getter) as well as being
	//  able to set and reset an initial angle/azimuth once this state is used as currently active state. In addition,
	//  we need to configure a tolerance within which we do nothing. We need pass this value once we return this state
	//  as the next state to have the original angles - we need getter and setter.
};

class LowSpeedForwardMotion : public ForwardMotion {
public:
	LowSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("LowSpeedForwardMotion", followUpState, fallbackState,
						  CLOSE_RANGE_MULTIPLIER, CLOSE_RANGE_MULTIPLIER, 0.5f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class MidSpeedForwardMotion : public ForwardMotion {
public:
	MidSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("MidSpeedForwardMotion", followUpState, fallbackState,
						  MID_RANGE_MULTIPLIER, MID_RANGE_MULTIPLIER, 1.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<Category::Direction, float> &minDistances,
					const std::map<Category::Direction, float> &maxDistances,
					const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override;
};

class FullSpeedForwardMotion : public ForwardMotion {
public:
	FullSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("FullSpeedForwardMotion", followUpState, fallbackState,
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
			MotionState(name, 0, 4, followUpState, fallbackState, left_speed, right_speed, 0.0f) {
		// nothing to do...
	};

	virtual bool delayedFollowUp(const std::map<Category::Direction, float> &minDistances,
								 const std::map<Category::Direction, float> &maxDistances,
								 const std::map<Category::Direction, float> &weightedMovingAvgDistances) const override {
		// TODO iterations are bad. We need to take the angle/azimuth difference from a starting angle/azimuth to the
		//  current angle/azimuth into account (e. g. whether we performed a 90° turn already or are still on the line)
		return _self_iterations <= k_max_self_iterations;
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
			return _followUpState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances) ||
				   _fallbackState->isEligible(minDistances, maxDistances, weightedMovingAvgDistances);
		}
	};
};

#endif // MOTION_STATE_H
