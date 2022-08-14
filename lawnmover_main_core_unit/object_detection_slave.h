#ifndef OBJECT_DETECTION_SLAVE_H
#define OBJECT_DETECTION_SLAVE_H

#include "master_spi_slave.h"

class ObjectDetectionSlave : public MasterSpiSlave {
public:
	ObjectDetectionSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const int slave_pin,
						 const int slave_restart_pin, const int slave_boot_delay, ESP32_PS4_Controller *esp32Ps4Ctrl,
						 const char *name = "ObjectDetectionSlave") :
			MasterSpiSlave(spi_slave_handler, slave_id, name, slave_pin, slave_restart_pin, slave_boot_delay, 3, 0) {
		// TODO set _data_request_callbacks and change the value passed to MasterSpiSlave
	};

	void fill_commands_bytes(uint8_t *tx_buffer) override {
		// TODO add object detection command for front, back and left/right distances
	};

	bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) override {
		return SpiCommands::master_interpret_communication(tx_buffer, slave_response_buffer, slave_response_buffer_size,
														   _data_request_callbacks, k_amount_data_request_callbacks);
	};
};

#endif // OBJECT_DETECTION_SLAVE_H
