#ifndef CATEGORY_H
#define CATEGORY_H

#include <numeric>
#include <vector>

#define OUT_OF_RANGE_MULTIPLIER (float) 1.0f
#define MID_RANGE_MULTIPLIER (float) 0.7f
#define CLOSE_RANGE_MULTIPLIER (float) 0.4f
#define CRITICAL_RANGE_MULTIPLIER (float) 0.25f
#define NOT_TOUCHED_MULTIPLIER (float) 0.1f

#define MAX_DISTANCE (float) 100.0f
#define MID_RANGE_LIMIT (float) MAX_DISTANCE * MID_RANGE_MULTIPLIER
#define CLOSE_RANGE_LIMIT (float) MAX_DISTANCE * CLOSE_RANGE_MULTIPLIER
#define CRITICAL_RANGE_LIMIT (float) MAX_DISTANCE * CRITICAL_RANGE_MULTIPLIER
#define NOT_TOUCHED_LIMIT (float) MAX_DISTANCE * NOT_TOUCHED_MULTIPLIER


class Category {
public:
	enum Distance {
		TOO_CLOSE,
		NOT_TOUCHED,
		CRITICAL_RANGE,
		CLOSE_RANGE,
		MID_RANGE,
		OUT_OF_RANGE
	};

	static Distance fromDistance(const float distance) {
		if (distance >= MAX_DISTANCE) {
			return OUT_OF_RANGE;
		} else if (distance >= MID_RANGE_LIMIT) {
			return MID_RANGE;
		} else if (distance >= CLOSE_RANGE_LIMIT) {
			return CLOSE_RANGE;
		} else if (distance >= CRITICAL_RANGE_LIMIT) {
			return CRITICAL_RANGE;
		} else if (distance >= NOT_TOUCHED_LIMIT) {
			return NOT_TOUCHED;
		} else {
			return TOO_CLOSE;
		}
	};

	static char *getNameFromDistance(const Distance &distance) {
		switch (distance) {
			case TOO_CLOSE:
				return "TooClose";
				break;
			case CRITICAL_RANGE:
				return "CriticalRange";
				break;
			case CLOSE_RANGE:
				return "CloseRange";
				break;
			case MID_RANGE:
				return "MidRange";
				break;
			case OUT_OF_RANGE:
				return "OutOfRange";
				break;
			case NOT_TOUCHED:
				return "NOT_TOUCHED";
				break;
			default:
				return "<unknown>";
		}
	};

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
private:
	Category() = delete;

	~Category() = delete;
};

class DirectionDistance {
public:
	DirectionDistance(const char *name, const int buffer_size, const float weightedMovingAverageAlpha) :
		k_name(name), k_buffer_size(buffer_size), k_weightedMovingAverageAlpha(weightedMovingAverageAlpha) {
		const float averageDefaultDistance = CLOSE_RANGE_LIMIT;
		SerialLogger::info("Creating %s distance buffers with size %d and add moving avg distances map entry with "
							"default of %d centimeters", name, k_buffer_size, averageDefaultDistance);
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

	float getMinDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return *std::min_element(_distances.begin(), _distances.end());
	};

	float getMaxDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return *std::max_element(_distances.begin(), _distances.end());
	};

	float getMeanDistance() const {
		// TODO the vector is volatile, is a copy necessary?!
		return std::accumulate(_distances.begin(), _distances.end(), 0.0f) / k_weightedMovingAverageAlpha;
	};

	std::vector<float> getDistances() const {
		return _distances;
	};

private:

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
