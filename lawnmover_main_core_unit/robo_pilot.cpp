#include "robo_pilot.h"
#include <serial_logger.h>

RuleBasedRoboPilot::RuleBasedRoboPilot() :
		RoboPilot("RuleBasedRoboPilot", 10) {
	// Nothing to do...
}

RuleBasedRoboPilot::~RuleBasedRoboPilot() = default;

MovementDecision RuleBasedRoboPilot::makeMovementDecision() {
	// TODO make real movement decision implementation
	return StopMovementDecision();
}