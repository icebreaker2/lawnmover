#ifndef OBJECT_DETECTION_CONTROLLER_H
#define OBJECT_DETECTION_CONTROLLER_H

#include "spi_slave_controller.h"

class ObjectDetectionController : public SpiSlaveController {
    public:
        ObjectDetectionController(const char *name = "ObjectDetectionController");

        void fill_commands_bytes(long &buffer_size, uint8_t *tx_buffer) override;
        bool consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer, long tx_buffer_size) override;

    private:
};

#endif // OBJECT_DETECTION_CONTROLLER_H
