#include "spi_commands.h"

int verifyIds(const byte rxIdBytes [], const byte txIdBytes []) {
    int16_t rxId = -1;
    int16_t txId = -1;

    memcpy(&rxId, rxIdBytes, sizeof(rxId));
    memcpy(&txId, txIdBytes, sizeof(txId));

    if (rxId == txId) {
        return rxId;
    } else {
        SerialLogger::error("Request and Acknowledge Id do not align: rx %d (%x%x) != tx %d(%x%x) ", rxId, rxIdBytes[0],
                            rxIdBytes[1], txId, txIdBytes[0], txIdBytes[1]);
        return -1;
    }
}

uint8_t SpiCommands::COMMUNICATION_START_SEQUENCE[COMMUNICATION_START_SEQUENCE_LENGTH] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xFF};

bool SpiCommands::master_interpret_communication(const uint8_t *tx_buffer, const uint8_t *rx_buffer, const long buffer_size) {
    SerialLogger::trace("Validating master-slave communication");
    uint8_t rxId1[COMMAND_FRAME_ID_SIZE];
    uint8_t rxId2[COMMAND_FRAME_ID_SIZE];
    uint8_t txId1_bytes[COMMAND_FRAME_ID_SIZE];
    uint8_t txId2_bytes[COMMAND_FRAME_ID_SIZE];
    uint8_t rx_value_bytes[COMMAND_FRAME_VALUE_SIZE];
    uint8_t tx_value_bytes[COMMAND_FRAME_VALUE_SIZE];

    for (long i = 0; i + COMMAND_FRAME_SIZE <= buffer_size; i += COMMAND_FRAME_SIZE) {
        // Get the ReqId
        for (int id_counter = 0; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
            //Serial.printf("%d, %d, %d -> %d: %x\n", COMMAND_SPI_RX_OFFSET , i, id_counter, COMMAND_SPI_RX_OFFSET + i + id_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + id_counter]);
            rxId1[id_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + id_counter];
            txId1_bytes[id_counter] = tx_buffer[i + id_counter];
        }

        // Get the command value
        for (int value_counter = 0; value_counter < COMMAND_FRAME_VALUE_SIZE; value_counter++) {
            //Serial.printf("%d: %x\n", COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + value_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + value_counter]);
            rx_value_bytes[value_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + value_counter];
            tx_value_bytes[value_counter] = tx_buffer[i + COMMAND_FRAME_ID_SIZE + value_counter];
        }

        // Get the AckId == ReqId
        for (int id_counter = 0; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
            //Serial.printf("%d: %x\n", COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter]);
            rxId2[id_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter];
            txId2_bytes[id_counter] = tx_buffer[i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter];
        }

        int16_t txId1 = 0;
        int16_t txId2 = 0;
        memcpy(&txId1, txId1_bytes, sizeof(txId1));
        memcpy(&txId2, txId2_bytes, sizeof(txId2));
        if (txId2 != -1) {
            SerialLogger::warn("Master did send wrong 2nd (ack) id which must be -1 but was %d. This is rather strange...", txId2);
        }

        const int16_t id1 = verifyIds(rxId1, txId1_bytes);
        // txId2 is the request to ack the very first id of a command, thus we use txId1 again to compare against rxId2
        const int16_t id2 = verifyIds(rxId2, txId1_bytes);
        SerialLogger::debug("Comparing %d (txId1) with %d (rxId1) with %d (rxId2/ackId) and value %x%x%x%x", txId1, id1, id2,
                            rx_value_bytes[0], rx_value_bytes[1], rx_value_bytes[2], rx_value_bytes[3]);
        if (id1 < 0 || id2 < 0) {
            return false;
        } else if (id1 != id2) {
            SerialLogger::error("Leading and Trailing Id do not align %d != %d", id1, id2);
            return false;
        } else if (id1 > MAX_ID) {
            SerialLogger::warn("Received bad id %d > %d (max)", id1, MAX_ID);
            return false;
        } else {
            long data_request = 0;
            memcpy(&data_request, rx_value_bytes, COMMAND_FRAME_VALUE_SIZE);
            if (data_request == -1) {
                SerialLogger::trace("Data request from slave");
                // TODO handle data received from slave in callback
                SerialLogger::warn("Cannot interpret value from data request to slave. Not yet implemented");
            } else {
                SerialLogger::trace("Data push to slave");
                for (int value_counter; value_counter < COMMAND_FRAME_VALUE_SIZE; value_counter++) {
                    if (rx_value_bytes[value_counter] != tx_value_bytes[value_counter]) {
                        SerialLogger::warn("Slave did not return correct value bytes");
                        return false;
                    }
                }
            }
        }
    }
    SerialLogger::trace("Validating master-slave communication done");
    return true;
}

int pos = 0;
uint8_t id_bytes [COMMAND_FRAME_ID_SIZE];
uint8_t value_bytes [COMMAND_FRAME_VALUE_SIZE];

bool SpiCommands::slave_process_partial_command(bool &synchronized, const uint8_t rx_byte, uint8_t &tx_byte) {
    if (synchronized) {
        if (pos < COMMAND_FRAME_ID_SIZE) {
            id_bytes[pos] = rx_byte;
            pos++;
            tx_byte = rx_byte;
        } else if (pos < COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE) {
            value_bytes[pos - COMMAND_FRAME_ID_SIZE] = rx_byte;
            tx_byte = rx_byte;
            pos++;
        } else if (pos < COMMAND_FRAME_SIZE - COMMAND_SPI_RX_OFFSET) {
            const int &id_index = pos % COMMAND_FRAME_ID_SIZE;
            if (rx_byte == 0xFF) {
                tx_byte = id_bytes[id_index];
                pos++;
            } else {
                //SerialLogger::warn("Received bad ack id request with byte %c on id index %d with position %d. Synchronization lost.", rx_byte, id_index, pos);
                tx_byte = 0;
                synchronized = false;
                pos = 0;
            }
        } else {
            // For the master to receive the nth byte we need to send a n+1 byte
            tx_byte = 0;
            pos = 0;
            return true;
        }
    } else {
        if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[pos]) {
            if (pos == COMMUNICATION_START_SEQUENCE_LENGTH - 1) {
                pos = 0;
                tx_byte = 0;
                if (rx_byte == 0xFF) {
                    synchronized = true;
                    return true;
                } else {
                    SerialLogger::warn("Bad end-of-sequence-byte received (%x) which should have been %x", rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[pos]);
                }
            } else {
                tx_byte = SpiCommands::COMMUNICATION_START_SEQUENCE[pos];
                pos++;
            }
        } else {
            pos = 0;
            //SerialLogger::warn("Lost synchronization. Received byte %c but expected %c at index %d. Restarting synchronization sequence.", rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[pos], pos);
        }
    }
    return false;
}
