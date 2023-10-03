#ifndef ENGINE_SLAVE_H
#define ENGINE_SLAVE_H

#include <robo_pilot.h>

#include "master_spi_slave.h"
#include "ESP32_PS4_Controller.h"

class EngineSlave : public MasterSpiSlave {
public:
	EngineSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const int slave_pin,
	            const int slave_restart_pin, ESP32_PS4_Controller *esp32Ps4Ctrl, RoboPilot *roboPilot,
	            const char *name = "EngineControl") :
			MasterSpiSlave(spi_slave_handler, slave_id, name, slave_pin, slave_restart_pin, 3, 0),
			_roboPilot(roboPilot) {
		_esp32Ps4Ctrl = esp32Ps4Ctrl;
	};

	void fill_commands_bytes(uint8_t *tx_buffer) override {
		const MovementDecision &movementDecision = _esp32Ps4Ctrl->isConnected() ?
		                                           MovementDecision(_esp32Ps4Ctrl->getLStickY(),
		                                                            _esp32Ps4Ctrl->getRStickY(),
		                                                            _esp32Ps4Ctrl->getRtValue()) :
		                                           (_roboPilot == nullptr ?
		                                            MovementDecision(0, 0, 0) : _roboPilot->makeMovementDecision());
		if (!_esp32Ps4Ctrl->isConnected()) {
			_roboPilot->printWeightedMovingAverageDistances();
		}

		SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, movementDecision.get_left_wheel_power(),
		                                tx_buffer);
		SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, movementDecision.get_right_wheel_power(),
		                                tx_buffer + COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, movementDecision.get_blade_motor_power(),
		                                tx_buffer + 2 * COMMAND_FRAME_SIZE);
	};

	bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) override {
		return interpret_communication(tx_buffer, slave_response_buffer, slave_response_buffer_size,
		                               _data_request_callbacks);
	};

private:
	ESP32_PS4_Controller *_esp32Ps4Ctrl;
	RoboPilot *_roboPilot;

	std::vector <std::function<bool(int16_t, int16_t)>> _data_request_callbacks;
};

#endif // ENGINE_SLAVE_H
