	#ifndef GYRO_H
#define GYRO_H

#include <Arduino.h>
#include <serial_logger.h>
// Based on https://github.com/pololu/lsm303-arduino
#include <Wire.h>
#include <LSM303.h>

class Gyroscope : public LSM303 {
public:
	enum Turn {
		LEFT = 0,
		RIGHT = 1
	};

	static const char *turn2String(const Turn turn) {
		if (turn == 0) {
			return "LEFT";
		} else if (turn == 1) {
			return "RIGHT";
		} else {
			SerialLogger::error(F("Unknown turn received: %i"), turn);
		}
	};

	Gyroscope() : LSM303() {
	    Wire.begin();
	    init();
	    enableDefault();
	    SerialLogger::debug(F("Initialized new Gyroscope with default arguments."));

	    _currentHeading = computeHeading();
	};

	float computeHeading() {
	    // Issue reading and store magnetometer values in m member (including x, y and z vectors)
		readMag();
		const float x = m.x;
	    const float y = m.y;
	    // Compute heading including tilt-compensation
		const float computedHeading = heading();
	    SerialLogger::trace(F("Computed compass heading %f based on x=%f and y=%f"), computedHeading, x, y);
	    return computedHeading;
	};

	static float getHeadingDiff(const float currentHeading, const float targetHeading) {
		const Headingdiff &diffs = computeDifferences(currentHeading, targetHeading);
		return min(diffs.left, diffs.right);
	};

	static Turn getHeadingTurn(const float currentHeading, const float targetHeading) {
		const Headingdiff &diffs = computeDifferences(currentHeading, targetHeading);
	    SerialLogger::trace(F("Computed compass heading differences of left=%f and right=%f"), diffs.left, diffs.right);
		if (diffs.left < diffs.right) {
			return LEFT;
		} else {
			return RIGHT;
		}
	};

private:
	struct Headingdiff {
		float left;
		float right;
	};

	static const struct Headingdiff computeDifferences(const float currentHeading, const float targetHeading) {
		const float simpleDiff = targetHeading - currentHeading;
		const float absDiff = abs(simpleDiff);
		struct Headingdiff diffs;
		if (currentHeading > targetHeading) {
			// issue with overrunning 360 for right turn
			diffs.right = (targetHeading + 360.0) - currentHeading;
			diffs.left = currentHeading - targetHeading;
		} else {
			// issue with overrunning 360 for left turn
			diffs.right = targetHeading - currentHeading;
			diffs.left = (currentHeading  + 360.0) - targetHeading;
		}
		return diffs;
	};

    const float k_Pi = 3.14159;
    float _currentHeading = 0.0;
};

#endif // GYRO_H
