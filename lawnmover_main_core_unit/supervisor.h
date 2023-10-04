#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include <robo_pilot.h>
#include "ESP32_PS4_Controller.h"

class Supervisor {
public:
	Supervisor(ESP32_PS4_Controller *esp32Ps4Ctrl, RoboPilot *roboPilot, const char *name = "Supervisor") :
			k_name(name), _esp32Ps4Ctrl(esp32Ps4Ctrl), _roboPilot(roboPilot) {
		// nothing to do...
	};

	MovementDecision decideMovement() {
		if (_esp32Ps4Ctrl == nullptr || _roboPilot == nullptr) {
            SerialLogger::warn(F("%s: Cannot determine correct movement. Ps4Ctrl or roboPilot not connected"), k_name);
		    return MovementDecision(0, 0, 0);
		} else {
            if (_esp32Ps4Ctrl->isConnected()) {
                SerialLogger::trace(F("%s: Overruling robo pilot with Ps4Ctrl"), k_name);
                return MovementDecision(_esp32Ps4Ctrl->getLStickY(), _esp32Ps4Ctrl->getRStickY(),
                                        _esp32Ps4Ctrl->getRtValue());
            } else {
                SerialLogger::trace(F("%s: Letting robo pilot decide the next movement"), k_name);
                return _roboPilot->getMovementDecision();
            }
		}
	};

private:
    const char *k_name;
    
	ESP32_PS4_Controller *_esp32Ps4Ctrl;
	RoboPilot *_roboPilot;
};

#endif // SUPERVISOR_H
