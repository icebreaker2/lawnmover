#ifndef SPI_SLAVE_CONTROLLER_H
#define SPI_SLAVE_CONTROLLER_H


#include <Arduino.h>

#include <stdint.h>
#include <spi_commands.h>
#include <serial_logger.h>

class SpiSlaveController {
    public:
        uint8_t *supply(long &buffer_size) {
            if (_slave_synchronized) {
                fill_commands_bytes(buffer_size, _tx_buffer);

                buffer_size = k_tx_buffer_size;
                return _tx_buffer;
            } else {
                buffer_size = COMMUNICATION_START_SEQUENCE_LENGTH;
                return SpiCommands::COMMUNICATION_START_SEQUENCE;
            }
        };

        bool consume(uint8_t *slave_response_buffer, long buffer_size) {
            bool valid = true;
            if (_slave_synchronized) {
                if (buffer_size != k_tx_buffer_size) {
                    SerialLogger::error("Cannot consume slave output from %s. Buffer size does not match written bytes to slave", k_name);
                    valid = false;
                } else {
                    valid = consume_commands(slave_response_buffer, buffer_size, _tx_buffer, k_tx_buffer_size);
                }
            } else {
                if (buffer_size != COMMUNICATION_START_SEQUENCE_LENGTH) {
                    SerialLogger::error("Cannot consume slave output from %s. Buffer size does not match written bytes to slave", k_name);
                    valid = false;
                } else {
                    // the latest (n+1) byte send is 0xFF which is needed to read the nth byte
                    for (long i = 0; i < COMMUNICATION_START_SEQUENCE_LENGTH - 1; i += 1) {
                        const char &rx_byte = slave_response_buffer[i + COMMAND_SPI_RX_OFFSET];
                        if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[i]) {
                            SerialLogger::debug("Slave from %s rx byte on index %d is %x and does match expected byte %x ", k_name, i,
                                                rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                        } else {
                            SerialLogger::warn("Slave from %s rx byte on index %d is %x and does not match expected byte %x ", k_name,
                                               i, rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                            valid = false;
                            break;
                        }
                    }
                }
            }

            if (valid) {
                if (!_slave_synchronized) {
                    SerialLogger::info("%s slave synchronized with this master", k_name);
                }
                _slave_synchronized = true;
            } else {
                if (_slave_synchronized) {
                    SerialLogger::info("%s slave no longer synchronized with this master", k_name);
                } else {
                    SerialLogger::warn("%s slave not synchronized with this master", k_name);
                }
                _slave_synchronized = false;
            }

            return valid;
        };

    protected:
        SpiSlaveController(const int amount_commands, const char *name) :
            k_tx_buffer_size(amount_commands * COMMAND_FRAME_SIZE), k_name(name) {
            _tx_buffer = new uint8_t[k_tx_buffer_size];
            _slave_synchronized = false;
        };

        ~SpiSlaveController() {
            delete [] _tx_buffer;
        };

        virtual void fill_commands_bytes(long &buffer_size, uint8_t *tx_buffer) = 0;
        virtual bool consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer, long tx_buffer_size) = 0;

    private:
        const char *k_name;
        const long k_tx_buffer_size;
        uint8_t *_tx_buffer;

        bool _slave_synchronized;
};

#endif // SPI_SLAVE_CONTROLLER_H
