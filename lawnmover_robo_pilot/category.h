#ifndef CATEGORY_H
#define CATEGORY_H

#define OUT_OF_RANGE_MULTIPLIER (float) 1.0f
#define MID_RANGE_MULTIPLIER (float) 0.7f
#define CLOSE_RANGE_MULTIPLIER (float) 0.5f
#define CRITICAL_RANGE_MULTIPLIER (float) 0.25f

#define MAX_DISTANCE (float) 100.0f
#define MID_RANGE_LIMIT (float) MAX_DISTANCE * MID_RANGE_MULTIPLIER
#define CLOSE_RANGE_LIMIT (float) MAX_DISTANCE * CLOSE_RANGE_MULTIPLIER
#define CRITICAL_RANGE_LIMIT (float) MAX_DISTANCE * CRITICAL_RANGE_MULTIPLIER


class Category {
public:
	enum Distance {
		TOO_CLOSE,
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
			default:
				return "<unknown>";
		}
	};

	enum Direction {
		FRONT,
		FRONT_LEFT,
		FRONT_RIGHT,
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

#endif // CATEGORY_H
