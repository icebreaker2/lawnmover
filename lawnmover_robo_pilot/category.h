#ifndef CATEGORY_H
#define CATEGORY_H

#include <numeric>
#include <vector>

#define OUT_OF_RANGE_PERCENTAGE (float) 1.0f
#define MID_RANGE_PERCENTAGE (float) 0.7f
#define CLOSE_RANGE_PERCENTAGE (float) 0.3f
#define CRITICAL_RANGE_PERCENTAGE (float) 0.15f

#define MAX_DISTANCE (float) 100.0f
#define MID_RANGE_LIMIT (float) MAX_DISTANCE * MID_RANGE_PERCENTAGE
#define CLOSE_RANGE_LIMIT (float) MAX_DISTANCE * CLOSE_RANGE_PERCENTAGE
#define CRITICAL_RANGE_LIMIT (float) MAX_DISTANCE * CRITICAL_RANGE_PERCENTAGE


class DirectionDistance {
public:
	enum Direction {
		FRONT,
		FRONT_LEFT,
		FRONT_RIGHT,
		LEFT,
		RIGHT,
		BACK_LEFT,
		BACK_RIGHT
	};

	static char *getNameFromDirection(const Direction &direction) {
		switch (direction) {
			case FRONT:
				return "FRONT";
				break;
			case FRONT_LEFT:
				return "FRONT_LEFT";
				break;
			case FRONT_RIGHT:
				return "FRONT_RIGHT";
				break;
			case LEFT:
				return "LEFT";
				break;
			case RIGHT:
				return "RIGHT";
				break;
			case BACK_LEFT:
				return "BACK_LEFT";
				break;
			case BACK_RIGHT:
				return "BACK_RIGHT";
				break;
			default:
				return "<unknown>";
		}
	};


	enum Distance {
		STOP,
		CRITICAL_RANGE,
		CLOSE_RANGE,
		MID_RANGE,
		OUT_OF_RANGE
	};

	static char *getNameFromDistance(const Distance &distance) {
		switch (distance) {
			case STOP:
				return "STOP";
				break;
			case CRITICAL_RANGE:
				return "CRITICAL_RANGE";
				break;
			case CLOSE_RANGE:
				return "CLOSE_RANGE";
				break;
			case MID_RANGE:
				return "MID_RANGE";
				break;
			case OUT_OF_RANGE:
				return "OUT_OF_RANGE";
				break;
			default:
				return "<unknown>";
		}
	};

	DirectionDistance(const char *name, const int buffer_size, const float weightedMovingAverageAlpha) :
		k_name(name), k_buffer_size(buffer_size), k_weightedMovingAverageAlpha(weightedMovingAverageAlpha) {
		const float averageDefaultDistance = CLOSE_RANGE_LIMIT;
		SerialLogger::info("Creating %s distance buffer with size %d and intial default of %f centimeters",
							name, k_buffer_size, averageDefaultDistance);
		// reserve space for the vector
		_distances.reserve(k_buffer_size);
		// fill initial values of weighted moving average and distances vector
		_movingAverageDistance = averageDefaultDistance;
		for (int i = 0; i < k_buffer_size; i++) {
			pushDistance(averageDefaultDistance);
		}
	};

	void pushDistance(const float distance) {
		_head = (_head + 1) % k_buffer_size;
		SerialLogger::trace(F("Pushing new distance for %s with value %f at index %d"), k_name, distance, _head);

		_distances[_head] = distance;
		updateMovingAverage(distance);
	};

	const char* getName() const { return k_name; };

	float getMovingAverageDistance() const {
		return _movingAverageDistance;
	};

	Distance getMovingAverageDistanceCategory() const {
		const float distance = getMovingAverageDistanceCategory();
		return fromDistance(distance);
	};

	float getMinDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return *std::min_element(_distances.begin(), _distances.end());
	};

	Distance getMinDistanceCategory() const {
		const float distance = getMinDistanceCategory();
		return fromDistance(distance);
	};

	float getMaxDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return *std::max_element(_distances.begin(), _distances.end());
	};

	Distance getMaxDistanceCategory() const {
		const float distance = getMaxDistanceCategory();
		return fromDistance(distance);
	};

	float getMeanDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return std::accumulate(_distances.begin(), _distances.end(), 0.0f) / k_weightedMovingAverageAlpha;
	};

	Distance getMeanDistanceCategory() const {
		const float distance = getMeanDistanceCategory();
		return fromDistance(distance);
	};

	std::vector<float> getDistances() const {
		return _distances;
	};

private:

	Distance fromDistance(const float distance) const {
		if (distance >= MAX_DISTANCE) {
			return OUT_OF_RANGE;
		} else if (distance >= MID_RANGE_LIMIT) {
			return MID_RANGE;
		} else if (distance >= CLOSE_RANGE_LIMIT) {
			return CLOSE_RANGE;
		} else if (distance >= CRITICAL_RANGE_LIMIT) {
			return CRITICAL_RANGE;
		} else {
			return STOP;
		}
	};

	void updateMovingAverage(const float distance) {
		_movingAverageDistance = k_weightedMovingAverageAlpha * _movingAverageDistance +
		                        (1 - k_weightedMovingAverageAlpha) * distance;
		SerialLogger::trace(F("Updated weighted moving average for %s to %f"), k_name, _movingAverageDistance);
	}

	const char *k_name;
	const int k_buffer_size;
	const float k_weightedMovingAverageAlpha;

	// TODO volatile?! --> not working with vector or map once you try to use their member functions or operators...
	std::vector<float> _distances;
	float _movingAverageDistance;

	int _head = -1;
};

#endif // CATEGORY_H
