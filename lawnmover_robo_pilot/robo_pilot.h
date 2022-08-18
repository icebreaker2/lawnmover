#ifndef ROBO_PILOT_H
#define ROBO_PILOT_H

#include <Arduino.h>
#include <numeric>

#include "decision.h"
#include "motion_state.h"

// TODO better algorithms https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance or see README

class RoboPilot {
public:
	RoboPilot(const char *name, const int distances_buffer_size,
			  const float weighted_moving_average_alpha = DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA) :
			k_name(name), k_distances_buffer_size(distances_buffer_size),
			k_weighted_moving_average_alpha(weighted_moving_average_alpha) {
		_directionsDistances.insert(std::make_pair(Category::Direction::FRONT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Category::Direction::FRONT_LEFT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Category::Direction::FRONT_RIGHT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Category::Direction::BACK_LEFT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Category::Direction::BACK_RIGHT, std::vector<float>()));

		// reserve space for the vectors
		_directionsDistances[Category::Direction::FRONT].reserve(distances_buffer_size);
		_directionsDistances[Category::Direction::FRONT_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Category::Direction::FRONT_RIGHT].reserve(distances_buffer_size);
		_directionsDistances[Category::Direction::BACK_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Category::Direction::BACK_RIGHT].reserve(distances_buffer_size);

		// fill initial values of weighted moving avergaes
		_weighted_moving_averages.insert(std::make_pair(Category::Direction::FRONT, 0));
		_weighted_moving_averages.insert(std::make_pair(Category::Direction::FRONT_LEFT, 0));
		_weighted_moving_averages.insert(std::make_pair(Category::Direction::FRONT_RIGHT, 0));
		_weighted_moving_averages.insert(std::make_pair(Category::Direction::BACK_LEFT, 0));
		_weighted_moving_averages.insert(std::make_pair(Category::Direction::BACK_RIGHT, 0));
	};

	~RoboPilot() = default;

	void putSensorDistance(Category::Direction direction, const float distance) {
		std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() >= k_distances_buffer_size) {
			directionDistances.pop_back();
		}
		directionDistances.insert(directionDistances.begin(), distance);

		updateSensorWeightedMovingAverage(direction, distance);
	}

	virtual MovementDecision makeMovementDecision() = 0;

protected:

	std::map<Category::Direction, float> getMeanSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = _weighted_moving_averages;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			const std::vector<float> &directionDistances = _directionsDistances.at(it->first);
			if (directionDistances.size() > 0) {
				it->second = std::accumulate(directionDistances.begin(), directionDistances.end(), 0) /
							 directionDistances.size();
			} else {
				it->second = 0.0f;
			}
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getMinSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = _weighted_moving_averages;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			const std::vector<float> &directionDistances = _directionsDistances.at(it->first);
			if (directionDistances.size() > 0) {
				it->second = *std::min_element(directionDistances.begin(), directionDistances.end());
			} else {
				it->second = 0.0f;
			}
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getMaxSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = _weighted_moving_averages;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			const std::vector<float> &directionDistances = _directionsDistances.at(it->first);
			if (directionDistances.size() > 0) {
				it->second = *std::max_element(directionDistances.begin(), directionDistances.end());
			} else {
				it->second = 0.0f;
			}
		}
		return copy_assigned_tmp_map;
	};

	std::map<Category::Direction, float> getWeightedMovingAverageSensorDistances() const {
		return _weighted_moving_averages;
	};

private:
	void updateSensorWeightedMovingAverage(Category::Direction direction, const float distance) {
		float &weightedMovingAverage = _weighted_moving_averages[direction];
		weightedMovingAverage = k_weighted_moving_average_alpha * distance +
								(1 - k_weighted_moving_average_alpha) * weightedMovingAverage;
	};

	const char *k_name;
	const int k_distances_buffer_size;
	// https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
	const float k_weighted_moving_average_alpha;

	// TODO volatile?! --> not working with vector or map once you try to use their member functions or operators...
	std::map <Category::Direction, std::vector<float>> _directionsDistances;
	std::map<Category::Direction, float> _weighted_moving_averages;
};

class RuleBasedMotionStateRoboPilot : public RoboPilot {
public:
	RuleBasedMotionStateRoboPilot();

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
