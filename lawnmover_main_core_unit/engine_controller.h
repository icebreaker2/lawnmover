#ifndef ENGINE_CONTROLLER_H
#define ENGINE_CONTROLLER_H

#include "spi_slave_controller.h"
#include "esp32_ps4_controller.h"

class EngineController : public SpiSlaveController {
    public:
        EngineController(ESP32_PS4_Controller *esp32Ps4Ctrl, const char *name = "EngineControl");

        void fill_commands_bytes(long &buffer_size, uint8_t *tx_buffer) override;
        bool consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer, long tx_buffer_size) override;

    private:
        ESP32_PS4_Controller *_esp32Ps4Ctrl;
};

#endif // ENGINE_CONTROLLER_H
