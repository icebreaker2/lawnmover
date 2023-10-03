#ifndef MOTION_STATE_H
#define MOTION_STATE_H

#define DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA (float) 0.50f
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
		if (fallbackState == nullptr && strcmp(k_name, "ErrorMotion") != 0) {
			SerialLogger::error(F("%s: nullptr fallback states are not allowed. Expect serious issues"), k_name);
		}
	};

	~MotionState() = default;

	/**
	 * Specialised class must determine whether the next state is the followUpState, this very state again (if
	 * eligible by check and by max_iterations) or if both of them are not possible the fallbackState
	 * @return
	 */
	virtual MotionState *getNextState(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) {
		_self_iterations++;
		if (_followUpState == nullptr) {
			if ((k_max_self_iterations < 0 || _self_iterations <= k_max_self_iterations) &&
			    this->isEligible(directionDistances)) {
				SerialLogger::trace(F("No follow up state and %s is (still) eligible, thus, returning it"), get_name());
				return this;
			} else {
				if (_self_iterations > 1) {
					SerialLogger::trace(F("%s no longer eligible, trying fallback %s due to nullptr followUpState and "
					                      "non-eligibility"), get_name(), _fallbackState->get_name());
				}
				return changeState(_fallbackState)->getNextState(directionDistances);
			}
		} else if (delayedFollowUp()) {
			// Note: We do not check the self iterations for good reasons. Assume we stop a left turn at a random step
			// t=3 while having reached an angle of 30/90°. If we now move to the fallback, we start with a wrong angle.
			// If we already use another function to determine eligibility, we need and can safely drop the check.
			if (this->isEligible(directionDistances)) {
				SerialLogger::trace(F("Returning %s due to delayed follow up and self-eligibility"), get_name());
				return this;
			} else {
				if (_self_iterations > 1) {
					SerialLogger::trace(F("%s no longer eligible, trying fallback %s due to delayed follow up but "
					                      "non-eligibility"), get_name(), _fallbackState->get_name());
				}
				return changeState(_fallbackState)->getNextState(directionDistances);
			}
		} else if (_followUpState->isEligible(directionDistances)) {
			SerialLogger::debug(F("Returning follow up state %s due to non-delayed follow up and follow-up states "
			                      "eligibility"), _followUpState->get_name());
			return changeState(_followUpState);
		} else {
			if ((k_max_self_iterations < 0 || _self_iterations <= k_max_self_iterations) &&
			    this->isEligible(directionDistances)) {
				SerialLogger::trace(
						F("Follow up state %s is not eligible but this %s is (still) eligible, thus, returning "
						  "it"), _followUpState->get_name(), get_name());
				return this;
			} else {
				if (_self_iterations > 1) {
					SerialLogger::trace(
							F("%s is no longer valide. Trying fallback %s due to non-delayed follow up and states non-eligibility"),
							get_name(), _fallbackState->get_name());
				}
				return changeState(_fallbackState)->getNextState(directionDistances);
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
	virtual bool delayedFollowUp() const {
		return _self_iterations <= k_min_self_iterations;
	};

	virtual bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const = 0;

protected:
	const int k_min_self_iterations;
	const int k_max_self_iterations;
	MotionState *_fallbackState;
	MotionState *_followUpState;

	int _self_iterations = 0;

private:
	MotionState *changeState(MotionState *nextState) {
		_self_iterations = 0;
		return nextState;
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

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class IdleMotion : public MotionState {
public:
	IdleMotion(MotionState *followUpState, MotionState *fallbackState) :
			MotionState("IdleMotion", 5, 10, followUpState, fallbackState, 0.0f, 0.0f, 0.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class ForwardMotion : public MotionState {
public:
	ForwardMotion(const char *name, MotionState *followUpState, MotionState *fallbackState,
	              const float &left_speed, const float &right_speed, const float blade_speed) :
			MotionState(name, 3, -1, followUpState, fallbackState, left_speed, right_speed, blade_speed) {
		// nothing to do...
	};

	// TODO once added heading/azitmuth we need to take this value into account for the left, right speed to keep on the
	//  line. Therefore, we need to be able to modify the return value (maybe overwrite the getter) as well as being
	//  able to set and reset an initial angle/azimuth once this state is used as currently active state. In addition,
	//  we need to configure a tolerance within which we do nothing. We need pass this value once we return this state
	//  as the next state to have the original angles - we need getter and setter.
};

class LowSpeedForwardMotion : public ForwardMotion {
public:
	LowSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("LowSpeedForwardMotion", followUpState, fallbackState,
			              CLOSE_RANGE_PERCENTAGE, CLOSE_RANGE_PERCENTAGE, 0.5f) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class MidSpeedForwardMotion : public ForwardMotion {
public:
	MidSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("MidSpeedForwardMotion", followUpState, fallbackState,
			              MID_RANGE_PERCENTAGE, MID_RANGE_PERCENTAGE, 1.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class FullSpeedForwardMotion : public ForwardMotion {
public:
	FullSpeedForwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			ForwardMotion("FullSpeedForwardMotion", followUpState, fallbackState,
			              OUT_OF_RANGE_PERCENTAGE, OUT_OF_RANGE_PERCENTAGE, 1.0f) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
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

	virtual bool delayedFollowUp() const override {
		// TODO iterations are bad. We need to take the angle/azimuth difference from a starting angle/azimuth to the
		//  current angle/azimuth into account (e. g. whether we performed a 90° turn already or are still on the line)
		return _self_iterations <= k_max_self_iterations;
	};
};

class LeftTurnMotion : public CollisionAvoidanceMotion {
public:
	LeftTurnMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("LeftTurnMotion", followUpState, fallbackState,
			                         -CLOSE_RANGE_PERCENTAGE, CLOSE_RANGE_PERCENTAGE) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class RightTurnMotion : public CollisionAvoidanceMotion {
public:
	RightTurnMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("RightTurnMotion", followUpState, fallbackState,
			                         CLOSE_RANGE_PERCENTAGE, -CLOSE_RANGE_PERCENTAGE) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};

class BackwardMotion : public CollisionAvoidanceMotion {
public:
	BackwardMotion(MotionState *followUpState, MotionState *fallbackState) :
			CollisionAvoidanceMotion("BackwardMotion", followUpState, fallbackState,
			                         -CLOSE_RANGE_PERCENTAGE, -CLOSE_RANGE_PERCENTAGE) {
		// nothing to do...
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override;
};


class PriorityStrategy : public MotionState {
public:
	PriorityStrategy(std::vector<MotionState *> &followUpStates, MotionState *fallbackState) :
			MotionState("PriorityStrategy", 0, 0, nullptr, fallbackState, 0.0f, 0.0f, 0.0f),
			_followUpStates(followUpStates) {
		// nothing to do...
	};

	/**
	 * Different strategy to get the next state based sorely on the followUpStates decision. This state is just for
	 * convenience.
	 *
	 * @return The next state based on this priority strategy
	 */
	MotionState *getNextState(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) override {
		if (_fallbackState == nullptr) {
			return nullptr;
		} else {
			for (MotionState *followUpState: _followUpStates) {
				if (followUpState->isEligible(directionDistances)) {
					return followUpState->getNextState(directionDistances);
				}
			}
			return _fallbackState->getNextState(directionDistances);
		}
	};

	bool isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const override {
		if (_fallbackState == nullptr) {
			return false;
		} else {
			bool eligible = false;
			for (MotionState *followUpState: _followUpStates) {
				if (followUpState->isEligible(directionDistances)) {
					eligible = true;
					break;
				}
			}
			return eligible || _fallbackState->isEligible(directionDistances);
		}
	};

private:
	std::vector<MotionState *> _followUpStates;
};

#endif // MOTION_STATE_H
