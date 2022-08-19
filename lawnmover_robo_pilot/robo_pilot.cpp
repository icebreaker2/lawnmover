#include "robo_pilot.h"

RuleBasedMotionStateRoboPilot::RuleBasedMotionStateRoboPilot() :
		RoboPilot("RuleBasedMotionStateRoboPilot", 10) {
	_errorMotion = new ErrorMotion();

	// Collision avoidance chaining
	_backwardMotion = new BackwardMotion(nullptr, _errorMotion);
	_leftTurnMotion = new LeftTurnMotion(nullptr, _backwardMotion);
	_rightTurnMotion = new RightTurnMotion(nullptr, _backwardMotion);

	_priorityStrategy = new PriorityStrategy(_leftTurnMotion, _rightTurnMotion);
	_idleMotion = new IdleMotion(nullptr, _priorityStrategy);

	// Cutting chaining
	_lowSpeedForwardMotion = new LowSpeedForwardMotion(nullptr, _idleMotion);
	_midSpeedForwardMotion = new MidSpeedForwardMotion(nullptr, _lowSpeedForwardMotion);
	_fullSpeedForwardMotion = new FullSpeedForwardMotion(nullptr, _midSpeedForwardMotion);

	_midSpeedForwardMotion->setFollowUpState(_fullSpeedForwardMotion);
	_lowSpeedForwardMotion->setFollowUpState(_midSpeedForwardMotion);

	// idle state and final loop chaining
	_idleMotion->setFollowUpState(_lowSpeedForwardMotion);
	_backwardMotion->setFollowUpState(_idleMotion);
	_leftTurnMotion->setFollowUpState(_idleMotion);
	_rightTurnMotion->setFollowUpState(_idleMotion);
	_errorMotion->setFollowUpState(_idleMotion);

	_currentMotion = _idleMotion;
}

RuleBasedMotionStateRoboPilot::~RuleBasedMotionStateRoboPilot() {
	delete _errorMotion;
	delete _backwardMotion;
	delete _leftTurnMotion;
	delete _rightTurnMotion;
	delete _priorityStrategy;
	delete _idleMotion;
	delete _lowSpeedForwardMotion;
	delete _midSpeedForwardMotion;
	delete _fullSpeedForwardMotion;
}

MovementDecision RuleBasedMotionStateRoboPilot::makeMovementDecision() {
	const char *last_name = _currentMotion->get_name();
	const std::map<Category::Direction, float> &minDistances = getMinSensorDistances();
	const std::map<Category::Direction, float> &maxDistances = getMaxSensorDistances();
	const std::map<Category::Direction, float> &weightedMovingAvgDistances = getWeightedMovingAverageSensorDistances();

	_currentMotion = _currentMotion->getNextState(minDistances, maxDistances, weightedMovingAvgDistances);
	if (_currentMotion == nullptr) {
		_currentMotion = _idleMotion;
		SerialLogger::error("MotionState chaining did not work and caused a next state that was nullptr. Last motion "
							"state was %s. Resetting to initial motion %s", last_name, _currentMotion->get_name());
		return StopMovementDecision();
	} else {
		SerialLogger::debug("Switched MotionState from %s to %s", last_name, _currentMotion->get_name());
		const MovementDecision &movementDecision = MovementDecision::fromState(*_currentMotion);
		return movementDecision;
	}
}