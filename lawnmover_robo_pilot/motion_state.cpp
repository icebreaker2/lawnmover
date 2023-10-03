#include "motion_state.h"

bool ErrorMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                             const std::map<DirectionDistance::Direction, float> &maxDistances,
                             const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	return true;
}

bool IdleMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                            const std::map<DirectionDistance::Direction, float> &maxDistances,
                            const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	return true;
}

bool LowSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                       const std::map<DirectionDistance::Direction, float> &maxDistances,
                                       const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));

	const bool eligible = frontDistance >= Category::CLOSE_RANGE &&
	                      frontLeftDistance >= Category::CLOSE_RANGE &&
	                      frontRightDistance >= Category::CLOSE_RANGE &&
	                      backLeftDistance >= Category::NOT_TOUCHED &&
	                      backRightDistance >= Category::NOT_TOUCHED;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}

bool MidSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                       const std::map<DirectionDistance::Direction, float> &maxDistances,
                                       const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));

	const bool eligible = frontDistance >= Category::MID_RANGE &&
	                      frontLeftDistance >= Category::CLOSE_RANGE &&
	                      frontRightDistance >= Category::CLOSE_RANGE &&
	                      backLeftDistance >= Category::NOT_TOUCHED &&
	                      backRightDistance >= Category::NOT_TOUCHED;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}

bool FullSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                        const std::map<DirectionDistance::Direction, float> &maxDistances,
                                        const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));

	const bool eligible = frontDistance >= Category::OUT_OF_RANGE &&
	                      frontLeftDistance >= Category::MID_RANGE &&
	                      frontRightDistance >= Category::MID_RANGE &&
	                      backLeftDistance >= Category::NOT_TOUCHED &&
	                      backRightDistance >= Category::NOT_TOUCHED;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}

bool BackwardMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                const std::map<DirectionDistance::Direction, float> &maxDistances,
                                const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));


	const bool eligible = backLeftDistance >= Category::CLOSE_RANGE &&
	                      backRightDistance >= Category::CLOSE_RANGE &&
	                      frontDistance >= Category::NOT_TOUCHED &&
	                      frontLeftDistance >= Category::NOT_TOUCHED &&
	                      frontRightDistance >= Category::NOT_TOUCHED;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}

bool LeftTurnMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                const std::map<DirectionDistance::Direction, float> &maxDistances,
                                const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));

	const bool eligible = frontDistance >= Category::CRITICAL_RANGE &&
	                      frontLeftDistance >= Category::CLOSE_RANGE &&
	                      frontRightDistance >= Category::CRITICAL_RANGE &&
	                      backLeftDistance >= Category::CLOSE_RANGE &&
	                      backRightDistance >= Category::CRITICAL_RANGE;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}

bool RightTurnMotion::isEligible(const std::map<DirectionDistance::Direction, float> &minDistances,
                                 const std::map<DirectionDistance::Direction, float> &maxDistances,
                                 const std::map<DirectionDistance::Direction, float> &weightedMovingAvgDistances) const {
	const Category::Distance &frontDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT));
	const Category::Distance &frontLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_LEFT));
	const Category::Distance &frontRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::FRONT_RIGHT));
	const Category::Distance &backLeftDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_LEFT));
	const Category::Distance &backRightDistance = Category::fromDistance(
			weightedMovingAvgDistances.at(DirectionDistance::Direction::BACK_RIGHT));

	const bool eligible = frontDistance >= Category::CRITICAL_RANGE &&
	                      frontLeftDistance >= Category::CRITICAL_RANGE &&
	                      frontRightDistance >= Category::CLOSE_RANGE &&
	                      backLeftDistance >= Category::CRITICAL_RANGE &&
	                      backRightDistance >= Category::CLOSE_RANGE;
	if (this->_self_iterations == 0) {
		if (eligible) {
			SerialLogger::debug(F("%s is eligible to be the next state. All ranges are above threshold"),
			                    get_name());
		} else {
			SerialLogger::debug(F("%s is NOT eligible to be the next state. Some ranges are below threshold"),
			                    get_name());
		}
	}
	return eligible;
}
