#ifndef ROBO_PILOT_H
#define ROBO_PILOT_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <numeric>

#include "decision.h"
#include "motion_state.h"

class RoboPilot {
public:
	enum Direction {
		FRONT,
		FRONT_LEFT,
		FRONT_RIGHT,
		BACK_LEFT,
		BACK_RIGHT
	};

	RoboPilot(const char *name, const int distances_buffer_size, const float weighted_moving_average_alpha = 0.70f) :
			k_name(name), k_distances_buffer_size(distances_buffer_size),
			k_weighted_moving_average_alpha(weighted_moving_average_alpha) {
		_directionsDistances.insert(std::make_pair(Direction::FRONT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Direction::FRONT_LEFT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Direction::FRONT_RIGHT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Direction::BACK_LEFT, std::vector<float>()));
		_directionsDistances.insert(std::make_pair(Direction::BACK_RIGHT, std::vector<float>()));

		// reserve space for the vectors
		_directionsDistances[Direction::FRONT].reserve(distances_buffer_size);
		_directionsDistances[Direction::FRONT_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Direction::FRONT_RIGHT].reserve(distances_buffer_size);
		_directionsDistances[Direction::BACK_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Direction::BACK_RIGHT].reserve(distances_buffer_size);

		// fill initial values of weighted moving avergaes
		_weighted_moving_averages.insert(std::make_pair(Direction::FRONT, 0));
		_weighted_moving_averages.insert(std::make_pair(Direction::FRONT_LEFT, 0));
		_weighted_moving_averages.insert(std::make_pair(Direction::FRONT_RIGHT, 0));
		_weighted_moving_averages.insert(std::make_pair(Direction::BACK_LEFT, 0));
		_weighted_moving_averages.insert(std::make_pair(Direction::BACK_RIGHT, 0));
	};

	~RoboPilot() = default;

	void putSensorDistance(Direction direction, const float distance) {
		std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() >= k_distances_buffer_size) {
			directionDistances.pop_back();
		}
		directionDistances.insert(directionDistances.begin(), distance);

		updateSensorWeightedMovingAverage(direction, distance);
	}

	virtual MovementDecision makeMovementDecision() = 0;

protected:

	float meanSensorDistance(Direction direction) {
		const std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return std::accumulate(directionDistances.begin(), directionDistances.end(), 0) / directionDistances.size();
		} else {
			return 0;
		}
	};

	float minSensorDistance(Direction direction) {
		const std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return *std::min_element(directionDistances.begin(), directionDistances.end());
		} else {
			return 0;
		}
	};

	float maxSensorDistance(Direction direction) {
		const std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return *std::max_element(directionDistances.begin(), directionDistances.end());
		} else {
			return 0;
		}
	};

private:
	void updateSensorWeightedMovingAverage(Direction direction, const float distance) {
		float &weightedMovingAverage = _weighted_moving_averages[direction];
		weightedMovingAverage = k_weighted_moving_average_alpha * distance +
								(1 - k_weighted_moving_average_alpha) * weightedMovingAverage;
	};

	const char *k_name;
	const int k_distances_buffer_size;
	// https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
	const float k_weighted_moving_average_alpha;

	// TODO volatile?! --> not working with vector or map once you try to use their member functions or operators...
	std::map <Direction, std::vector<float>> _directionsDistances;
	std::map<Direction, float> _weighted_moving_averages;
};

class RuleBasedRoboPilot : public RoboPilot {
public:
	RuleBasedRoboPilot();

	~RuleBasedRoboPilot();

	MovementDecision makeMovementDecision() override;

private:
	ErrorMotion *_errorMotion;
	BackwardMotion *_backwardMotion;
	LeftTurnMotion *_leftTurnMotion;
	RightTurnMotion *_rightTurnMotion;
	IdleMotion *_idleMotion;
	LowSpeedForwardMotion *_lowSpeedForwardMotion;
	MidSpeedForwardMotion *_midSpeedForwardMotion;
	FullSpeedForwardMotion *_fullSpeedForwardMotion;
};

#endif // ROBO_PILOT_H
