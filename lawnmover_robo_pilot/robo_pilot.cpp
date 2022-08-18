#include "robo_pilot.h"

RuleBasedRoboPilot::RuleBasedRoboPilot() :
		RoboPilot("RuleBasedRoboPilot", 10) {
	_errorMotion = new ErrorMotion();

	// Collision avoidance chaining
	_backwardMotion = new BackwardMotion(nullptr, _errorMotion);
	_leftTurnMotion = new LeftTurnMotion(nullptr, _backwardMotion);
	_rightTurnMotion = new RightTurnMotion(nullptr, _backwardMotion);

	// TODO enable multiple fallbackStates strategies {_leftTurnMotion, _rightTurnMotion}
	_idleMotion = new IdleMotion(nullptr, _leftTurnMotion);

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
}

RuleBasedRoboPilot::~RuleBasedRoboPilot() {
	delete _errorMotion;
	delete _backwardMotion;
	delete _leftTurnMotion;
	delete _rightTurnMotion;
	delete _idleMotion;
	delete _lowSpeedForwardMotion;
	delete _midSpeedForwardMotion;
	delete _fullSpeedForwardMotion;
}

// TODO better algorithms https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance
MovementDecision RuleBasedRoboPilot::makeMovementDecision() {
	// TODO make real movement decision implementation
	return StopMovementDecision();
}