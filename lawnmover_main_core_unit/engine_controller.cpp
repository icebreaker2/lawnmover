#include "engine_controller.h"

#include <Arduino.h>
#include <serial_logger.h>


EngineController::EngineController(ESP32_PS4_Controller *esp32Ps4Ctrl) : SpiSlaveController(3)  {
    _esp32Ps4Ctrl = esp32Ps4Ctrl;
}

uint8_t *EngineController::supply(long &buffer_size) {
    if (getSynchronizationState()) {
        SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getLStickY(), _tx_buffer);
        SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getRStickY(), _tx_buffer + COMMAND_FRAME_SIZE);
        SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, _esp32Ps4Ctrl->getRtValue(), _tx_buffer + 2 * COMMAND_FRAME_SIZE);

        buffer_size = k_tx_buffer_size;
        return _tx_buffer;
    } else {
        buffer_size = COMMUNICATION_START_SEQUENCE_LENGTH;
        return SpiCommands::COMMUNICATION_START_SEQUENCE;
    }
}

bool EngineController::consume(uint8_t *slave_response_buffer, long buffer_size) {
    bool valid = true;
    bool &slave_synchronized = getSynchronizationState();
    Serial.print("RxBufferInput:");
    for (long i = 0; i < buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(slave_response_buffer[i], HEX);
    }
    Serial.println();
    Serial.print("TxBufferInput:");
    for (long i = 0; i < buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(_tx_buffer[i], HEX);
    }
    Serial.println();

    if (slave_synchronized) {
        if (buffer_size != k_tx_buffer_size) {
            SerialLogger::error("Cannot consume slave output. Buffer size does not match written bytes to slave");
            valid = false;
        } else {
            valid = SpiCommands::master_interpret_communication(_tx_buffer, slave_response_buffer, buffer_size);
        }
    } else {
        if (buffer_size != COMMUNICATION_START_SEQUENCE_LENGTH) {
            SerialLogger::error("Cannot consume slave output. Buffer size does not match written bytes to slave");
            valid = false;
        } else {
            // the latest (n+1) byte send is 0xFF which is needed to read the nth byte
            for (long i = 0; i < COMMUNICATION_START_SEQUENCE_LENGTH - 1; i += 1) {
                const char &rx_byte = slave_response_buffer[i + COMMAND_SPI_RX_OFFSET];
                if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[i]) {
                    SerialLogger::debug("Slave rx byte on index %d is %x and does match expected byte %x ", i, rx_byte,
                                        SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                } else {
                    SerialLogger::warn("Slave rx byte on index %d is %x and does not match expected byte %x ", i, rx_byte,
                                       SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                    valid = false;
                    break;
                }
            }
        }
    }

    if (valid) {
        if (!slave_synchronized) {
            SerialLogger::info("Engine Slave synchronized with this master");
        }
        slave_synchronized = true;
    } else {
        if (slave_synchronized) {
            SerialLogger::info("Engine Slave no longer synchronized with this master");
        } else {
            SerialLogger::info("Engine Slave not synchronized with this master");
        }
        slave_synchronized = false;
    }

    return valid;
}
