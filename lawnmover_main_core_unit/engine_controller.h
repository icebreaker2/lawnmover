#ifndef ENGINE_CONTROLLER_H
#define ENGINE_CONTROLLER_H

#include "spi_slave_controller.h"
#include "esp32_ps4_controller.h"

class EngineController : public SpiSlaveController {
    public:
        EngineController(ESP32_PS4_Controller *esp32Ps4Ctrl);

        uint8_t *supply(long &buffer_size) override;
        bool consume(uint8_t *slave_response_buffer, long buffer_size) override;

    private:
        ESP32_PS4_Controller *_esp32Ps4Ctrl;
};

#endif // ENGINE_CONTROLLER_H
