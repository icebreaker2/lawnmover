#include "motion_state.h"

bool ErrorMotion::isEligible() const {
	return false;
}

bool IdleMotion::isEligible() const {
	return false;
}

bool LowSpeedForwardMotion::isEligible() const {
	return false;
}

bool MidSpeedForwardMotion::isEligible() const {
	return false;
}

bool FullSpeedForwardMotion::isEligible() const {
	return false;
}

bool BackwardMotion::isEligible() const {
	return false;
}

bool LeftTurnMotion::isEligible() const {
	return false;
}

bool RightTurnMotion::isEligible() const {
	return false;
}