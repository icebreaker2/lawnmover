#ifndef ENGINE_SLAVE_H
#define ENGINE_SLAVE_H

#include "master_spi_slave.h"
#include "ESP32_PS4_Controller.h"

class EngineSlave : public MasterSpiSlave {
public:
	EngineSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const int slave_pin, const int slave_restart_pin,
				const int slave_boot_delay, ESP32_PS4_Controller *esp32Ps4Ctrl, const char *name = "EngineControl") :
			MasterSpiSlave(spi_slave_handler, slave_id, name, slave_pin, slave_restart_pin, slave_boot_delay, 3, 0) {
		_esp32Ps4Ctrl = esp32Ps4Ctrl;
	};

	void fill_commands_bytes(uint8_t *tx_buffer) override {
		SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getLStickY(), tx_buffer);
		SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getRStickY(),
										tx_buffer + COMMAND_FRAME_SIZE);
		SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, _esp32Ps4Ctrl->getRtValue(),
										tx_buffer + 2 * COMMAND_FRAME_SIZE);
	};

	bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) override {
		return SpiCommands::master_interpret_communication(tx_buffer, slave_response_buffer, slave_response_buffer_size,
														   _data_request_callbacks, k_amount_data_request_callbacks);
	};

private:
	ESP32_PS4_Controller *_esp32Ps4Ctrl;
};

#endif // ENGINE_SLAVE_H
