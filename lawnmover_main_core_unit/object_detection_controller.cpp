#include "object_detection_controller.h"

ObjectDetectionController::ObjectDetectionController(const char *name) : SpiSlaveController(0, name)  {
    // nothing to do here...
}

void ObjectDetectionController::fill_commands_bytes(long &buffer_size, uint8_t *tx_buffer) {
    // TODO add object detection command
    // SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getLStickY(), tx_buffer);
    SerialLogger::info("Filling %l bytes into buffer", buffer_size);
}

bool ObjectDetectionController::consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer, long tx_buffer_size) {
    Serial.print("RxBufferInput:");
    for (long i = 0; i < slave_response_buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(slave_response_buffer[i], HEX);
    }
    Serial.println();
    Serial.print("TxBufferInput:");
    for (long i = 0; i < tx_buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(tx_buffer[i], HEX);
    }
    Serial.println();

    // TODO implement interpretation
    return true;
}
