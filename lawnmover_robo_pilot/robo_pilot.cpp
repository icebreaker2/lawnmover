#include "robo_pilot.h"
#include <serial_logger.h>

RuleBasedRoboPilot::RuleBasedRoboPilot() :
		RoboPilot("RuleBasedRoboPilot", 10) {
	// Nothing to do...
}

RuleBasedRoboPilot::~RuleBasedRoboPilot() = default;

// TODO better algorithms https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance
MovementDecision RuleBasedRoboPilot::makeMovementDecision() {
	// TODO make real movement decision implementation
	return StopMovementDecision();
}