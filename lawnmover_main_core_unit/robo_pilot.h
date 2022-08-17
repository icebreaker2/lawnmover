#ifndef ROBO_PILOT_H
#define ROBO_PILOT_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <numeric>
#include "decision.h"

class RoboPilot {
public:
	enum Direction {
		FRONT,
		FRONT_LEFT,
		FRONT_RIGHT,
		BACK_LEFT,
		BACK_RIGHT
	};

	RoboPilot(const char *name, const int distances_buffer_size) :
			k_name(name), k_distances_buffer_size(distances_buffer_size) {
		_directionsDistances.insert(std::make_pair(Direction::FRONT, std::vector<long>()));
		_directionsDistances.insert(std::make_pair(Direction::FRONT_LEFT, std::vector<long>()));
		_directionsDistances.insert(std::make_pair(Direction::FRONT_RIGHT, std::vector<long>()));
		_directionsDistances.insert(std::make_pair(Direction::BACK_LEFT, std::vector<long>()));
		_directionsDistances.insert(std::make_pair(Direction::BACK_RIGHT, std::vector<long>()));

		// reserve space for the vectors
		_directionsDistances[Direction::FRONT].reserve(distances_buffer_size);
		_directionsDistances[Direction::FRONT_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Direction::FRONT_RIGHT].reserve(distances_buffer_size);
		_directionsDistances[Direction::BACK_LEFT].reserve(distances_buffer_size);
		_directionsDistances[Direction::BACK_RIGHT].reserve(distances_buffer_size);
	};

	~RoboPilot();

	void putSensorDistance(Direction direction, const long distance) {
		std::vector<long> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() >= k_distances_buffer_size) {
			directionDistances.pop_back();
		}
		directionDistances.insert(directionDistances.begin(), distance);
	}

	virtual MovementDecision makeMovementDecision() = 0;

protected:

	long meanSensorDistance(Direction direction) {
		const std::vector<long> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return std::accumulate(directionDistances.begin(), directionDistances.end(), 0) / directionDistances.size();
		} else {
			return 0;
		}
	};

	long minSensorDistance(Direction direction) {
		const std::vector<long> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return *std::min_element(directionDistances.begin(), directionDistances.end());
		} else {
			return 0;
		}
	};

	long maxSensorDistance(Direction direction) {
		const std::vector<long> &directionDistances = _directionsDistances[direction];
		if (directionDistances.size() > 0) {
			return *std::max_element(directionDistances.begin(), directionDistances.end());
		} else {
			return 0;
		}
	};

private:
	const char *k_name;
	const int k_distances_buffer_size;

    // TODO volatile?! --> not working with vector or map once you try to use their member functions or operators...
	std::map <Direction, std::vector<long>> _directionsDistances;
};

class RuleBasedRoboPilot : public RoboPilot {
public:
	RuleBasedRoboPilot();

	~RuleBasedRoboPilot();

	MovementDecision makeMovementDecision() override;

private:
};

#endif // ROBO_PILOT_H
