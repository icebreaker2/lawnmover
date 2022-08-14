#ifndef OBSTACLE_DETECTION_SLAVE_H
#define OBSTACLE_DETECTION_SLAVE_H

#include "master_spi_slave.h"

class ObstacleDetectionSlave : public MasterSpiSlave {
public:
	ObstacleDetectionSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const int slave_pin,
						 const int slave_restart_pin, const int slave_boot_delay, 
						 const char *name = "ObstacleDetectionSlave") :
			MasterSpiSlave(spi_slave_handler, slave_id, name, slave_pin, slave_restart_pin, slave_boot_delay, 0, 0) {
	   // TODO set _data_request_callbacks and change the value passed to MasterSpiSlave
       // TODO set amount commands correctly and set them in fill_commands_bytes
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

#endif // OBSTACLE_DETECTION_SLAVE_H
