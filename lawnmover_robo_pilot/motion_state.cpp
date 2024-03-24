#include "motion_state.h"

bool ErrorMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	return true;
}

bool IdleMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	return true;
}

bool LowSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::CLOSE_RANGE &&
		           frontLeftDistance >= DirectionDistance::CLOSE_RANGE &&
		           frontRightDistance >= DirectionDistance::CLOSE_RANGE &&
		           leftDistance >= DirectionDistance::CRITICAL_RANGE &&
		           rightDistance >= DirectionDistance::CRITICAL_RANGE &&
		           backLeftDistance >= DirectionDistance::STOP &&
		           backRightDistance >= DirectionDistance::STOP;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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

bool MidSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::MID_RANGE &&
				   frontLeftDistance >= DirectionDistance::CLOSE_RANGE &&
		           frontRightDistance >= DirectionDistance::CLOSE_RANGE &&
	               leftDistance >= DirectionDistance::CRITICAL_RANGE &&
		           rightDistance >= DirectionDistance::CRITICAL_RANGE &&
		           backLeftDistance >= DirectionDistance::STOP &&
		           backRightDistance >= DirectionDistance::STOP;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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

bool FullSpeedForwardMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::OUT_OF_RANGE &&
				   frontLeftDistance >= DirectionDistance::MID_RANGE &&
		           frontRightDistance >= DirectionDistance::MID_RANGE &&
	               leftDistance >= DirectionDistance::CLOSE_RANGE &&
		           rightDistance >= DirectionDistance::CLOSE_RANGE &&
		           backLeftDistance >= DirectionDistance::STOP &&
		           backRightDistance >= DirectionDistance::STOP;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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

bool BackwardMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::STOP &&
		           frontLeftDistance >= DirectionDistance::STOP &&
		           frontRightDistance >= DirectionDistance::STOP;
				   leftDistance >= DirectionDistance::CRITICAL_RANGE &&
		           rightDistance >= DirectionDistance::CRITICAL_RANGE &&
	               backLeftDistance >= DirectionDistance::CLOSE_RANGE &&
		           backRightDistance >= DirectionDistance::CLOSE_RANGE;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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

bool LeftTurnMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::CRITICAL_RANGE &&
				   frontLeftDistance >= DirectionDistance::CLOSE_RANGE &&
		           frontRightDistance >= DirectionDistance::CRITICAL_RANGE &&
	               leftDistance >= DirectionDistance::CLOSE_RANGE &&
		           rightDistance >= DirectionDistance::CRITICAL_RANGE &&
		           backLeftDistance >= DirectionDistance::STOP &&
		           backRightDistance >= DirectionDistance::STOP;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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

bool RightTurnMotion::isEligible(const std::map<DirectionDistance::Direction, DirectionDistance*> &directionDistances) const {
	bool eligible = false;
	try {
		const DirectionDistance::Distance &frontDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontLeftDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &frontRightDistance =
				directionDistances.at(DirectionDistance::Direction::FRONT_RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &leftDistance =
				directionDistances.at(DirectionDistance::Direction::LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &rightDistance =
				directionDistances.at(DirectionDistance::Direction::RIGHT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backLeftDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_LEFT)->getMovingAverageDistanceCategory();
		const DirectionDistance::Distance &backRightDistance =
				directionDistances.at(DirectionDistance::Direction::BACK_RIGHT)->getMovingAverageDistanceCategory();

		eligible = frontDistance >= DirectionDistance::CRITICAL_RANGE &&
				   frontLeftDistance >= DirectionDistance::CRITICAL_RANGE &&
		           frontRightDistance >= DirectionDistance::CLOSE_RANGE &&
	               leftDistance >= DirectionDistance::CRITICAL_RANGE &&
		           rightDistance >= DirectionDistance::CLOSE_RANGE &&
		           backLeftDistance >= DirectionDistance::STOP &&
		           backRightDistance >= DirectionDistance::STOP;
	} catch (std::out_of_range &e) {
		SerialLogger::error(F("Cannot determine eligibility. Missing a key in DirectionDistances: %s"), e.what());
	}

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
