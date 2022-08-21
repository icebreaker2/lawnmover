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

uint8_t SpiCommands::COMMUNICATION_START_SEQUENCE[COMMUNICATION_START_SEQUENCE_LENGTH] = {0x08, 0x07, 0x06, 0x05, 0x04,
																						  0x03, 0x02, 0x01, 0xFF};