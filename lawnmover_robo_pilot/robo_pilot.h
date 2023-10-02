#ifndef ROBO_PILOT_H
#define ROBO_PILOT_H

#include <Arduino.h>
#include <numeric>
#include <vector>

#include "decision.h"
#include "motion_state.h"

// TODO better algorithms https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance or see README

class RoboPilot {
public:
	RoboPilot(const char *name, const int distances_buffer_size, const std::vector<Category::Direction> &directions,
	          const float weighted_moving_average_alpha = DEFAULT_WEIGHTED_MOVING_AVERAGE_ALPHA) :
			k_name(name), k_distances_buffer_size(distances_buffer_size),
			k_weighted_moving_average_alpha(weighted_moving_average_alpha) {
		for (const Category::Direction &direction : directions) {
			const float averageDefaultDistance = CLOSE_RANGE_LIMIT;
			SerialLogger::info("Creating %s distance buffers with size %d and add moving avg distances map entry with "
								"default of %d centimeters", Category::getNameFromDirection(direction),
								k_distances_buffer_size, averageDefaultDistance);
            // insert new direction
			_directionsDistances.insert(std::make_pair(direction, std::vector<float>()));
			// reserve space for the vectors
			_directionsDistances[direction].reserve(k_distances_buffer_size);
		    // fill initial values of weighted moving averages
			_weighted_moving_averages.insert(std::make_pair(direction, averageDefaultDistance));
		}
	};

	~RoboPilot() = default;

	void putSensorDistance(Category::Direction direction, const float distance) {
		std::vector<float> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() >= k_distances_buffer_size) {
			SerialLogger::trace(F("Removing oldest distance from %s with value %f"),
			                    Category::getNameFromDirection(direction), directionDistances.back());
			directionDistances.pop_back();
		}
		SerialLogger::trace(F("Inserting new distance for %s with value %f"),
		                    Category::getNameFromDirection(direction), distance);
		directionDistances.insert(directionDistances.begin(), distance);

		updateSensorWeightedMovingAverage(direction, distance);
	};

	virtual MovementDecision makeMovementDecision() = 0;

	void printWeightedMovingAverageDistances() const {
		if (SerialLogger::isBelow(SerialLogger::DEBUG)) {
			for (auto it = _weighted_moving_averages.begin(); it != _weighted_moving_averages.end(); ++it) {
				Serial.print(Category::getNameFromDirection(it->first));
				Serial.print(F(": "));
				Serial.print(it->second);
				Serial.print(F(", "));
			}
			Serial.println();
		}
	};

protected:

	std::map<Category::Direction, float> getMeanSensorDistances() const {
		std::map<Category::Direction, float> copy_assigned_tmp_map = _weighted_moving_averages;
		for (auto it = copy_assigned_tmp_map.begin(); it != copy_assigned_tmp_map.end(); ++it) {
			const std::vector<float> &directionDistances = _directionsDistances.at(it->first);
			if (directionDistances.size() > 0) {
				it->second = std::accumulate(directionDistances.begin(), directionDistances.end(), 0.0f) /
				             directionDistances.size();
			} else {
				it->second = -1.0f;
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
				it->second = -1.0f;
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
				it->second = -1.0f;
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
		SerialLogger::trace(F("Updated weighted moving average for %s to %f"),
		                    Category::getNameFromDirection(direction), weightedMovingAverage);
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
