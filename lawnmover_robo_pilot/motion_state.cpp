#include "motion_state.h"

/**
 * If there is a large discrepancy between the min distance of a Direction and the weighted moving average, we need to
 * take safety measures because
 * * maybe the exponential weight of the average is to strong, thus, reducing the min in recursive definition
 *   very fast while benefiting more recent distances
 * * maybe the history list is too long keeping very old values for too long or the update frequency too short leading
 *   to many observations in a long list where the moving average will weight them down exponentially
 * * General sensor malfunctioning
 *
 * Please note: Min distance will always be lower or equal than the weighted moving average
 *
 * @return whether there is a discrepancy in one of the front directions larger than configured or not
 */
bool has_large_min_wAvg_differences(const float minDistance, const float weightedMovingAvgDistance) {
	const float &min_wAvg_discrepancy = minDistance / weightedMovingAvgDistance;
	return min_wAvg_discrepancy > DEFAULT_WEIGHTED_MOVING_AVERAGE_MIN_MAX_DISCREPANCY;
}

bool has_large_min_wAvg_differences_front(const std::map<Category::Direction, float> &minDistances,
										  const std::map<Category::Direction, float> &weightedMovingAvgDistances) {
	return has_large_min_wAvg_differences(minDistances.at(Category::FRONT_LEFT),
										  weightedMovingAvgDistances.at(Category::FRONT_LEFT)) ||
		   has_large_min_wAvg_differences(minDistances.at(Category::FRONT),
										  weightedMovingAvgDistances.at(Category::FRONT)) ||
		   has_large_min_wAvg_differences(minDistances.at(Category::FRONT_RIGHT),
										  weightedMovingAvgDistances.at(Category::FRONT_RIGHT));
}

bool has_large_min_wAvg_differences_back(const std::map<Category::Direction, float> &minDistances,
										 const std::map<Category::Direction, float> &weightedMovingAvgDistances) {
	return has_large_min_wAvg_differences(minDistances.at(Category::BACK_LEFT),
										  weightedMovingAvgDistances.at(Category::BACK_LEFT)) ||
		   has_large_min_wAvg_differences(minDistances.at(Category::BACK_RIGHT),
										  weightedMovingAvgDistances.at(Category::BACK_RIGHT));
}

bool ErrorMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
							 const std::map<Category::Direction, float> &maxDistances,
							 const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return true;
}

bool IdleMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
							const std::map<Category::Direction, float> &maxDistances,
							const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return true;
}

bool LowSpeedForwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
									   const std::map<Category::Direction, float> &maxDistances,
									   const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	if (has_large_min_wAvg_differences_front(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between front min and wAvg distances to high. Need cleaner history for save "
						   "eligibility.");
		return false;
	} else {
		const Category::Distance &frontDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT));
		const Category::Distance &frontLeftDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_LEFT));
		const Category::Distance &frontRightDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_RIGHT));

		const bool eligible = frontDistance >= Category::CLOSE_RANGE && frontLeftDistance >= Category::CLOSE_RANGE &&
							  frontRightDistance >= Category::CLOSE_RANGE;
		if (eligible) {
			SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
		} else {
			SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
								get_name());
		}
		return eligible;
	}
}

bool MidSpeedForwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
									   const std::map<Category::Direction, float> &maxDistances,
									   const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	if (has_large_min_wAvg_differences_front(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between front min and wAvg distances to high. Need cleaner history for save "
						   "eligibility.");
		return false;
	} else {
		const Category::Distance &frontDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT));
		const Category::Distance &frontLeftDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_LEFT));
		const Category::Distance &frontRightDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_RIGHT));

		const bool eligible = frontDistance >= Category::MID_RANGE && frontLeftDistance >= Category::CLOSE_RANGE &&
							  frontRightDistance >= Category::CLOSE_RANGE;
		if (eligible) {
			SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
		} else {
			SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
								get_name());
		}
		return eligible;
	}
}

bool FullSpeedForwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
										const std::map<Category::Direction, float> &maxDistances,
										const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	if (has_large_min_wAvg_differences_front(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between front min and wAvg distances to high. Need cleaner history for save "
						   "eligibility.");
		return false;
	} else {
		const Category::Distance &frontDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT));
		const Category::Distance &frontLeftDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_LEFT));
		const Category::Distance &frontRightDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::FRONT_RIGHT));

		const bool eligible = frontDistance >= Category::OUT_OF_RANGE && frontLeftDistance >= Category::MID_RANGE &&
							  frontRightDistance >= Category::MID_RANGE;
		if (eligible) {
			SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
		} else {
			SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
								get_name());
		}
		return eligible;
	}
}

bool BackwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								const std::map<Category::Direction, float> &maxDistances,
								const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	if (has_large_min_wAvg_differences_back(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between backwards min and wAvg distances to high. Need cleaner history for "
						   "save eligibility.");
		return false;
	} else {
		const Category::Distance &backLeftDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::BACK_LEFT));
		const Category::Distance &backRightDistance = Category::fromDistance(
				weightedMovingAvgDistances.at(Category::BACK_RIGHT));

		const bool eligible = backLeftDistance >= Category::CLOSE_RANGE && backRightDistance >= Category::CLOSE_RANGE;
		if (eligible) {
			SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
		} else {
			SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
								get_name());
		}
		return eligible;
	}
}

bool LeftTurnMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								const std::map<Category::Direction, float> &maxDistances,
								const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	// If we want to turn, we need clean history in front and backwards sensors
	if (has_large_min_wAvg_differences_front(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between front min and wAvg distances to high. Need cleaner history for save "
						   "eligibility.");
		return false;
	} else {
		if (has_large_min_wAvg_differences_back(minDistances, weightedMovingAvgDistances)) {
			SerialLogger::warn("Discrepancy between backwards min and wAvg distances to high. Need cleaner history for "
							   "save eligibility.");
			return false;
		} else {
			const Category::Distance &frontDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT));
			const Category::Distance &frontLeftDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT_LEFT));
			const Category::Distance &frontRightDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT_RIGHT));
			const Category::Distance &backLeftDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::BACK_LEFT));
			const Category::Distance &backRightDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::BACK_RIGHT));

			const bool eligible = frontDistance >= Category::CRITICAL_RANGE &&
								  frontLeftDistance >= Category::CLOSE_RANGE &&
								  frontRightDistance >= Category::CRITICAL_RANGE &&
								  backLeftDistance >= Category::CLOSE_RANGE &&
								  backRightDistance >= Category::CRITICAL_RANGE;
			if (eligible) {
				SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
			} else {
				SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
									get_name());
			}
			return eligible;
		}
	}
}

bool RightTurnMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								 const std::map<Category::Direction, float> &maxDistances,
								 const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	// If we want to turn, we need clean history in front and backwards sensors
	if (has_large_min_wAvg_differences_front(minDistances, weightedMovingAvgDistances)) {
		SerialLogger::warn("Discrepancy between front min and wAvg distances to high. Need cleaner history for save "
						   "eligibility.");
		return false;
	} else {
		if (has_large_min_wAvg_differences_back(minDistances, weightedMovingAvgDistances)) {
			SerialLogger::warn("Discrepancy between backwards min and wAvg distances to high. Need cleaner history for "
							   "save eligibility.");
			return false;
		} else {
			const Category::Distance &frontDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT));
			const Category::Distance &frontLeftDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT_LEFT));
			const Category::Distance &frontRightDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::FRONT_RIGHT));
			const Category::Distance &backLeftDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::BACK_LEFT));
			const Category::Distance &backRightDistance = Category::fromDistance(
					weightedMovingAvgDistances.at(Category::BACK_RIGHT));

			const bool eligible = frontDistance >= Category::CRITICAL_RANGE &&
								  frontLeftDistance >= Category::CRITICAL_RANGE &&
								  frontRightDistance >= Category::CLOSE_RANGE &&
								  backLeftDistance >= Category::CRITICAL_RANGE &&
								  backRightDistance >= Category::CLOSE_RANGE;
			if (eligible) {
				SerialLogger::debug("%s is eligible to be the next state. All ranges are above threshold", get_name());
			} else {
				SerialLogger::debug("%s is NOT eligible to be the next state. Some ranges are below threshold",
									get_name());
			}
			return eligible;
		}
	}
}