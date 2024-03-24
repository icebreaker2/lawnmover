#ifndef ROBO_PILOT_H
#define ROBO_PILOT_H

#include <Arduino.h>
#include <numeric>
#include <vector>

#include "decision.h"
#include "motion_state.h"
#include "category.h"

// TODO better algorithms https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance or see README

class RoboPilot {
public:
	RoboPilot(const char *name, const int distances_buffer_size, const std::vector<DirectionDistance::Direction> &directions,
	          const float weighted_moving_average_alpha = DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA) :
			k_name(name) {
		for (const DirectionDistance::Direction &direction : directions) {
			_directionsDistances.insert(std::make_pair(direction,
										new DirectionDistance(DirectionDistance::getNameFromDirection(direction),
										distances_buffer_size, weighted_moving_average_alpha)));
		}
	};

	~RoboPilot() {
		for (auto it = _directionsDistances.begin(); it != _directionsDistances.end(); ++it) {
			delete it->second;
		}
	};

	void putSensorDistance(DirectionDistance::Direction direction, const float distance) {
		_directionsDistances[direction]->pushDistance(distance);
	};

	void printWeightedMovingAverageDistances() const {
		Serial.print(F("MovingAverages -> "));
		for (auto it = _directionsDistances.begin(); it != _directionsDistances.end(); ++it) {
			Serial.print(it->second->getName());
			Serial.print(F(":"));
			Serial.print(it->second->getMovingAverageDistance());
			Serial.print(F(", "));
		}
		Serial.println();
	};

	virtual MovementDecision makeMovementDecision() = 0;

	MovementDecision getMovementDecision() {
		if (SerialLogger::isBelow(SerialLogger::DEBUG)) {
			printWeightedMovingAverageDistances();
		}
		return makeMovementDecision();
	};

protected:
	std::map<DirectionDistance::Direction, DirectionDistance*> _directionsDistances;

private:
	const char *k_name;
};

class RuleBasedMotionStateRoboPilot : public RoboPilot {
public:
	RuleBasedMotionStateRoboPilot(const std::vector<DirectionDistance::Direction> &directions);

	~RuleBasedMotionStateRoboPilot();

	MovementDecision makeMovementDecision() override;

private:
	ErrorMotion *_errorMotion;
	BackwardMotion *_backwardMotion;
	LeftTurnMotion *_leftTurnMotion;
	RightTurnMotion *_rightTurnMotion;
	PriorityStrategy *_priorityStrategy;
	IdleMotion *_idleMotion;
	LowSpeedForwardMotion *_lowSpeedForwardMotion;
	MidSpeedForwardMotion *_midSpeedForwardMotion;
	FullSpeedForwardMotion *_fullSpeedForwardMotion;

	MotionState *_currentMotion;
};

#endif // ROBO_PILOT_H
