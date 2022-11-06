#include "spi_commands.h"

int SpiCommands::verifyIds(const byte rxIdBytes[], const byte txIdBytes[]) {
	int16_t rxId = -1;
	int16_t txId = -1;

	memcpy(&rxId, rxIdBytes, sizeof(rxId));
	memcpy(&txId, txIdBytes, sizeof(txId));

	if (rxId == txId) {
		return rxId;
	} else {
		SerialLogger::error(F("Request and Acknowledge Id do not align: rx %d (%x%x) != tx %d(%x%x)"), rxId,
		                    rxIdBytes[0], rxIdBytes[1], txId, txIdBytes[0], txIdBytes[1]);
		return -1;
	}
}

const char *SpiCommands::getNameFromId(const int16_t id) {
	switch (id) {
		case LEFT_WHEEL_STEERING_COMMAND:
			return "LEFT_WHEEL_STEERING";
		case RIGHT_WHEEL_STEERING_COMMAND :
			return "RIGHT_WHEEL_STEERING";
		case MOTOR_SPEED_COMMAND:
			return "MOTOR_SPEED";
		case OBSTACLE_FRONT_COMMAND :
			return "OBSTACLE_FRONT";
		case OBSTACLE_FRONT_LEFT_COMMAND :
			return "OBSTACLE_FRONT_LEFT";
		case OBSTACLE_FRONT_RIGHT_COMMAND :
			return "OBSTACLE_FRONT_RIGHT";
		case OBSTACLE_BACK_LEFT_COMMAND :
			return "OBSTACLE_BACK_LEFT";
		case OBSTACLE_BACK_RIGHT_COMMAND :
			return "OBSTACLE_BACK_RIGHT";
		default:
			return "<unknown>";
	}
}

uint8_t SpiCommands::COMMUNICATION_START_SEQUENCE[COMMUNICATION_START_SEQUENCE_LENGTH] = {0x08, 0x07, 0x06, 0x05, 0x04,
                                                                                          0x03, 0x02, 0x01, 0xFF};