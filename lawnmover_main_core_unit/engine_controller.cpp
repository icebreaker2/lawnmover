#include "engine_controller.h"

EngineController::EngineController(ESP32_PS4_Controller *esp32Ps4Ctrl, const char *name) : SpiSlaveController(3, name)  {
    _esp32Ps4Ctrl = esp32Ps4Ctrl;
}

void EngineController::fill_commands_bytes(long &buffer_size, uint8_t *tx_buffer) {
    SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getLStickY(), tx_buffer);
    SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, _esp32Ps4Ctrl->getRStickY(), tx_buffer + COMMAND_FRAME_SIZE);
    SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, _esp32Ps4Ctrl->getRtValue(), tx_buffer + 2 * COMMAND_FRAME_SIZE);
}

bool EngineController::consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer, long tx_buffer_size) {
    return SpiCommands::master_interpret_communication(tx_buffer, slave_response_buffer, slave_response_buffer_size);
}
