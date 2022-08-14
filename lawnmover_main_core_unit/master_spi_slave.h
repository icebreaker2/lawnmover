#ifndef LAWNMOVER_UTILS_SPI_SLAVE_H
#define LAWNMOVER_UTILS_SPI_SLAVE_H

#include <Arduino.h>
#include <stdint.h>

#include <spi_commands.h>
#include <serial_logger.h>

#include "spi_slave_handler.h"

class MasterSpiSlave {
public:
    MasterSpiSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const char *name, const int slave_pin,
             const int slave_restart_pin, const int slave_boot_delay, const int amount_commands,
             const int amount_data_request_callbacks) :             
      k_slave_id(slave_id), k_name(name), k_slave_pin(slave_pin), k_slave_restart_pin(slave_restart_pin),
      k_slave_boot_delay(slave_boot_delay), k_amount_commands(amount_commands),
      k_amount_data_request_callbacks(amount_data_request_callbacks),
      k_buffer_size(amount_commands * COMMAND_FRAME_SIZE) {
      _spi_slave_handler = spi_slave_handler;

      // to use DMA buffer, use these methods to allocate buffer
      _tx_buffer = (uint8_t *) malloc(k_buffer_size * sizeof _tx_buffer);
      memset(_tx_buffer, 0, k_buffer_size);

      // to use DMA buffer, use these methods to allocate buffer
      _rx_buffer = (uint8_t *) malloc(k_buffer_size * sizeof _rx_buffer);
      memset(_rx_buffer, 0, k_buffer_size);

      delay(1000);

      _data_request_callbacks = (data_request_callback *) malloc(
                                  k_amount_data_request_callbacks * sizeof _data_request_callbacks);
    };

	uint8_t *supply(long &buffer_size) {
		if (_slave_synchronized) {
			fill_commands_bytes(_tx_buffer);

			buffer_size = k_buffer_size;
			return _tx_buffer;
		} else {
			buffer_size = COMMUNICATION_START_SEQUENCE_LENGTH;
			return SpiCommands::COMMUNICATION_START_SEQUENCE;
		}
	};

	bool consume(uint8_t *slave_response_buffer, long buffer_size) {
		bool valid = true;
		if (_slave_synchronized) {
			if (buffer_size != k_buffer_size) {
				SerialLogger::error("Cannot consume slave output from %s. Buffer size does not match written "
									"bytes to slave", k_name);
				valid = false;
			} else {
				valid = consume_commands(slave_response_buffer, buffer_size, _tx_buffer);
			}
		} else {
			if (buffer_size != COMMUNICATION_START_SEQUENCE_LENGTH) {
				SerialLogger::error(
						"Cannot consume slave output from %s. Buffer size does not match written bytes to slave",
						k_name);
				valid = false;
			} else {
				// the latest (n+1) byte send is 0xFF which is needed to read the nth byte
				for (long i = 0; i < COMMUNICATION_START_SEQUENCE_LENGTH - 1; i += 1) {
					const char &rx_byte = slave_response_buffer[i + COMMAND_SPI_RX_OFFSET];
					if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[i]) {
						SerialLogger::debug("Slave from %s rx byte on index %d is %x and does match expected byte %x ",
											k_name, i, rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
					} else {
						SerialLogger::warn(
								"Slave from %s rx byte on index %d is %x and does not match expected byte %x ", k_name,
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


	void restart() {
		SerialLogger::info("(Re)Starting slave %d (%s) connected to slave-select pin %d with power supply on pin %d",
						   k_slave_id + 1, k_name, k_slave_pin, k_slave_restart_pin);
		// if we put some voltage on the reset pin, the board will restart
		pinMode(k_slave_restart_pin, OUTPUT);
		digitalWrite(k_slave_restart_pin, HIGH);
		delay(100);
		digitalWrite(k_slave_restart_pin, LOW);

		// wait some time for the slave to boot
		SerialLogger::info("Powering up slave on slave_select pin %d (%s) with reset on pin %d", k_slave_pin,
						   k_name, k_slave_restart_pin);
		delay(k_slave_boot_delay);
		SerialLogger::info("Slave %d needs to synchronize with this master", k_slave_id + 1);
	};

	int get_slave_id() const {
		return k_slave_id;
	};

	const char *get_name() const {
		return k_name;
	};

	int get_slave_pin() const {
		return k_slave_pin;
	};

	int get_slave_restart_pin() const {
		return k_slave_restart_pin;
	};

	int get_slave_boot_delay() const {
		return k_slave_boot_delay;
	};

	int get_amount_commands() const {
		return k_amount_commands;
	};

	int get_amount_data_request_callbacks() const {
		return k_amount_data_request_callbacks;
	};

	long get_buffer_size() const {
		return k_buffer_size;
	};

	bool is_slave_synchronized() const {
		return _slave_synchronized;
	};

	SpiSlaveHandler *get_spi_slave_handler() const {
		return _spi_slave_handler;
	};

protected:
	virtual void fill_commands_bytes(uint8_t *tx_buffer) = 0;

	virtual bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) = 0;

	const int k_amount_data_request_callbacks;
	data_request_callback *_data_request_callbacks;

private:
	const int k_slave_id;
	const char *k_name;
	const int k_slave_pin;
	const int k_slave_restart_pin;
	const int k_slave_boot_delay;
	const int k_amount_commands;
	const long k_buffer_size;

	uint8_t *_tx_buffer;
	uint8_t *_rx_buffer;

	bool _slave_synchronized;
	SpiSlaveHandler *_spi_slave_handler;
};

#endif //LAWNMOVER_UTILS_SPI_SLAVE_H
