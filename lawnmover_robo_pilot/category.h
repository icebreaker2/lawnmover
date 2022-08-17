#ifndef CATEGORY_H
#define CATEGORY_H

#define OUT_OF_RANGE_MULTIPLIER (float) 1.0f
#define MID_RANGE_MULTIPLIER (float) 0.7f
#define CLOSE_RANGE_MULTIPLIER (float) 0.45f
#define CRITICAL_RANGE_MULTIPLIER (float) 20.0f

#define MAX_DISTANCE (float) 103.0f
#define MID_RANGE_LIMIT (float) MAX_DISTANCE * MID_RANGE_MULTIPLIER
#define CLOSE_RANGE_LIMIT (float) MAX_DISTANCE * CLOSE_RANGE_MULTIPLIER


class DistanceCategory {
public:
	enum Category {
		CRITICAL_RANGE,
		CLOSE_RANGE,
		MID_RANGE,
		OUT_OF_RANGE
	};

	static Category fromDistance(const float distance) {
		if (distance >= MAX_DISTANCE) {
			return OUT_OF_RANGE;
		} else if (distance >= MID_RANGE_LIMIT) {
			return MID_RANGE;
		} else if (distance >= CLOSE_RANGE_LIMIT) {
			return CLOSE_RANGE;
		} else {
			return CRITICAL_RANGE;
		}
	};

	static char *getNameFromCategory(const Category &category) {
		switch (category) {
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
			default:
				return "<unknown>";
		}
	};
private:
	DistanceCategory() = delete;

	~DistanceCategory() = delete;
};

#endif // CATEGORY_H
