#ifndef SPI_SLAVE_CONTROLLER_H
#define SPI_SLAVE_CONTROLLER_H

#include <stdint.h>
#include <spi_commands.h>

class SpiSlaveController {
    public:
        virtual uint8_t *supply(long &buffer_size) = 0;
        virtual bool consume(uint8_t *slave_response_buffer, long buffer_size) = 0;

        bool &getSynchronizationState() {
            return _slave_synchronized;
        };

    protected:
        SpiSlaveController(const int amount_commands) : k_tx_buffer_size(amount_commands * (COMMAND_FRAME_SIZE)) {
            _tx_buffer = new uint8_t[k_tx_buffer_size];
            _slave_synchronized = false;
        };

        ~SpiSlaveController() {
            delete [] _tx_buffer;
        };

        const long k_tx_buffer_size;
        uint8_t *_tx_buffer;

    private:
        bool _slave_synchronized;
};

#endif // SPI_SLAVE_CONTROLLER_H
