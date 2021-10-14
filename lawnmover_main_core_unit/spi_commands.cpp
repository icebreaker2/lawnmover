#include "spi_commands.h"

#include <serial_logger.h>

int verifyIds(const byte rxIdBytes [], const byte txIdBytes []) {
    int rxId = -1;
    int txId = -1;

    memcpy(&rxId, rxIdBytes, sizeof(int16_t));
    memcpy(&txId, txIdBytes, sizeof(int16_t));

    if (rxId == txId) {
        return rxId;
    } else {
        SerialLogger::error("Request and Acknowledge Id do not align %d != %d", rxId, txId);
        return -1;
    }
}

bool SpiCommands::master_interpret_communication(const uint8_t *tx_buffer, const uint8_t *rx_buffer, const long buffer_size) {
    SerialLogger::trace("Validating master-slave communication");
    byte rxId1[COMMAND_FRAME_ID_SIZE];
    byte rxId2[COMMAND_FRAME_ID_SIZE];
    byte txId1[COMMAND_FRAME_ID_SIZE];
    byte txId2[COMMAND_FRAME_ID_SIZE];
    uint8_t rx_value_bytes[COMMAND_FRAME_VALUE_SIZE];
    uint8_t tx_value_bytes[COMMAND_FRAME_VALUE_SIZE];

    for (long i = 0; i + COMMAND_FRAME_SIZE <= buffer_size; i += COMMAND_FRAME_SIZE) {
        for (int id_counter; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
            rxId1[id_counter] = rx_buffer[id_counter];
            txId1[id_counter] = tx_buffer[id_counter];
        }


        for (int value_counter; value_counter < COMMAND_FRAME_VALUE_SIZE; value_counter++) {
            rx_value_bytes[value_counter] = rx_buffer[COMMAND_FRAME_ID_SIZE + value_counter];
            tx_value_bytes[value_counter] = tx_buffer[COMMAND_FRAME_ID_SIZE + value_counter];
        }

        for (int id_counter; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
            rxId2[id_counter] = rx_buffer[COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter];
            txId2[id_counter] = tx_buffer[COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter];
        }

        int16_t txId = 0;
        memcpy(&txId, txId2, sizeof(int16_t));
        if (txId != -1) {
            SerialLogger::warn("Master did send wrong 2nd (ack) id which must be -1 but was %d. This is rather strange...", txId);
        }

        const int16_t id1 = verifyIds(rxId1, txId1);
        const int16_t id2 = verifyIds(rxId2, txId1);
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

/**
    Call to interpret Id send by master
*/
int16_t SpiCommands::slave_interpret_command_id(const uint8_t *rx_buffer) {
    int16_t id = -1;
    byte rxId[COMMAND_FRAME_ID_SIZE];

    for (int id_counter; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
        rxId[id_counter] = rx_buffer[id_counter];
    }

    memcpy(&id, rxId, sizeof(int16_t));
    if (id < 0) {
        SerialLogger::error("Bad Id Received. %d is unknown", id);
    } else if (id > MAX_ID) {
        SerialLogger::warn("Received bad id %d > %d (max)", id, MAX_ID);
    }
    return id;
}

/*
    Call if value received by master

    Note: Function-Pointer may differ if request only received. If so, slave must write values to tx_buffer
*/
bool SpiCommands::slave_interpret_command(const int16_t id, uint8_t *rx_buffer, uint8_t *tx_buffer,
        bool (*leftWheelSteeringCommand)(float), bool (*rightWheelSteeringCommand)(float), bool (*motorSpeedCommand)(int16_t)) {
    bool valid = false;
    switch (id) {
        case LEFT_WHEEL_STEERING_COMMAND: {
                const float value = FloatSpiCommand::interpretBytes(rx_buffer);
                valid = leftWheelSteeringCommand(value);
                break;
            }
        case RIGHT_WHEEL_STEERING_COMMAND: {
                const float value = FloatSpiCommand::interpretBytes(rx_buffer);
                valid = rightWheelSteeringCommand(value);
                break;
            }
        case MOTOR_SPEED_COMMAND: {
                const int16_t value = IntegerSpiCommand::interpretBytes(rx_buffer);
                valid = motorSpeedCommand(value);
                break;
            }
        default:
            SerialLogger::warn("Unknown command received with identifier: %d", id);
    }
    return valid;
}
