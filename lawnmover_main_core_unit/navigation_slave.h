#ifndef NAVIGATION_SLAVE_H
#define NAVIGATION_SLAVE_H

#include <robo_pilot.h>

#include "master_spi_slave.h"

class NavigationSlave : public MasterSpiSlave {
public:
	NavigationSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const int slave_pin,
	                       const int slave_restart_pin, RoboPilot *roboPilot,
	                       const char *name = "NavigationSlave") :
			MasterSpiSlave(spi_slave_handler, slave_id, name, slave_pin, slave_restart_pin, 0, NAVIGATION_COMMANDS),
			_roboPilot(roboPilot) {
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_FRONT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::FRONT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_FRONT_LEFT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::FRONT_LEFT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_FRONT_RIGHT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::FRONT_RIGHT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_LEFT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::LEFT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_RIGHT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::RIGHT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_BACK_LEFT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::BACK_LEFT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == OBSTACLE_BACK_RIGHT_COMMAND && _roboPilot != nullptr) {
				_roboPilot->putSensorDistance(DirectionDistance::Direction::BACK_RIGHT, distance);
				return true;
			} else {
				return false;
			}
		});
		_data_request_callbacks.push_back([&](int16_t id, float distance) -> bool {
			if (id == GYRO_COMMAND && _roboPilot != nullptr) {
				// TODO interpret results
				return true;
			} else {
				return false;
			}
		});
	};

	void fill_commands_bytes(uint8_t *tx_buffer) override {
		SpiCommands::putCommandToBuffer(OBSTACLE_FRONT_COMMAND, DATA_REQUEST_VALUE_BYTES, tx_buffer);
		SpiCommands::putCommandToBuffer(OBSTACLE_FRONT_LEFT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(OBSTACLE_FRONT_RIGHT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 2 * COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(OBSTACLE_LEFT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 3 * COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(OBSTACLE_RIGHT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 4 * COMMAND_FRAME_SIZE);
        SpiCommands::putCommandToBuffer(OBSTACLE_BACK_LEFT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 5 * COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(OBSTACLE_BACK_RIGHT_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 6 * COMMAND_FRAME_SIZE);
        SpiCommands::putCommandToBuffer(GYRO_COMMAND, DATA_REQUEST_VALUE_BYTES,
		                                tx_buffer + 7 * COMMAND_FRAME_SIZE);
	};

	bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) override {
		return interpret_communication(tx_buffer, slave_response_buffer, slave_response_buffer_size,
		                               _data_request_callbacks);
	};

private:
	RoboPilot *_roboPilot;
	std::vector <std::function<bool(int16_t, float)>> _data_request_callbacks;
};

#endif // NAVIGATION_SLAVE_H
