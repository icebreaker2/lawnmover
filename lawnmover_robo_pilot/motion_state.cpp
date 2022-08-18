#include "motion_state.h"

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
	return false;
}

bool MidSpeedForwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
									   const std::map<Category::Direction, float> &maxDistances,
									   const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return false;
}

bool FullSpeedForwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
										const std::map<Category::Direction, float> &maxDistances,
										const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return false;
}

bool BackwardMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								const std::map<Category::Direction, float> &maxDistances,
								const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return false;
}

bool LeftTurnMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								const std::map<Category::Direction, float> &maxDistances,
								const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return false;
}

bool RightTurnMotion::isEligible(const std::map<Category::Direction, float> &minDistances,
								 const std::map<Category::Direction, float> &maxDistances,
								 const std::map<Category::Direction, float> &weightedMovingAvgDistances) const {
	return false;
}