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
	RoboPilot(const char *name, const int distances_buffer_size, const std::vector<Category::Direction> &directions,
	          const float weighted_moving_average_alpha = DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA) :
			k_name(name) {
		for (const Category::Direction &direction : directions) {
			_directionsDistances.insert(std::make_pair(direction,
										new DirectionDistance(Category::getNameFromDirection(direction),
										distances_buffer_size, weighted_moving_average_alpha)));
			copyMap.insert(std::make_pair(direction, -1.0));
		}
	};

	~RoboPilot() {
		for (auto it = _directionsDistances.begin(); it != _directionsDistances.end(); ++it) {
			delete it->second;
		}
	};

	void putSensorDistance(Category::Direction direction, const float distance) {
		_directionsDistances[direction]->pushDistance(distance);
	};

	void printWeightedMovingAverageDistances() const {
		if (SerialLogger::isBelow(SerialLogger::DEBUG)) {
			Serial.print(F("MovingAverage of "));
			for (auto it = _directionsDistances.begin(); it != _directionsDistances.end(); ++it) {
				Serial.print(it->second->getName());
				Serial.print(F("is:"));
				Serial.print(it->second->getMovingAverageDistance());
				Serial.print(F(", of "));
			}
			Serial.println();
		}
	};

	virtual MovementDecision makeMovementDecision() = 0;

protected:

	std::map<Category::Direction, float> getMeanSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = copyMap;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			it->second = _directionsDistances.at(it->first)->getMeanDistance();
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getMinSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = copyMap;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			it->second = _directionsDistances.at(it->first)->getMinDistance();
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getMaxSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = copyMap;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			it->second = _directionsDistances.at(it->first)->getMaxDistance();;
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getWeightedMovingAverageSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = copyMap;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			it->second = _directionsDistances.at(it->first)->getMovingAverageDistance();;
		}
		return copy_assigned_tmp_map;
	};

private:
	const char *k_name;
	std::map<Category::Direction, DirectionDistance*> _directionsDistances;
	std::map<Category::Direction, float> copyMap;
};

class RuleBasedMotionStateRoboPilot : public RoboPilot {
public:
	RuleBasedMotionStateRoboPilot(const std::vector<Category::Direction> &directions);

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
