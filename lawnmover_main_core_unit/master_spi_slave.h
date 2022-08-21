#ifndef LAWNMOVER_UTILS_SPI_SLAVE_H
#define LAWNMOVER_UTILS_SPI_SLAVE_H

#include <Arduino.h>
#include <stdint.h>

#include <vector>
#include <functional>

#include <spi_commands.h>
#include <serial_logger.h>


#include "spi_slave_handler.h"

class MasterSpiSlave {
public:
	MasterSpiSlave(SpiSlaveHandler *spi_slave_handler, const int slave_id, const char *name, const int slave_pin,
				   const int slave_restart_pin, const int amount_data_push_commands,
				   const int amount_data_request_commands) :
			k_slave_id(slave_id), k_name(name), k_slave_pin(slave_pin), k_slave_restart_pin(slave_restart_pin),
			k_amount_data_push_commands(amount_data_push_commands),
			k_amount_data_request_callbacks(amount_data_request_commands),
			k_buffer_size((amount_data_push_commands + amount_data_request_commands) * COMMAND_FRAME_SIZE) {
		_spi_slave_handler = spi_slave_handler;

		// to use DMA buffer, use these methods to allocate buffer
		_tx_buffer = (uint8_t *) malloc(k_buffer_size * sizeof _tx_buffer);
		memset(_tx_buffer, 0, k_buffer_size);

		// to use DMA buffer, use these methods to allocate buffer
		_rx_buffer = (uint8_t *) malloc(k_buffer_size * sizeof _rx_buffer);
		memset(_rx_buffer, 0, k_buffer_size);

		delay(1000);
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

	uint8_t *get_rx_buffer(long tx_rx_buffer_size) {
		if (tx_rx_buffer_size > k_buffer_size) {
			SerialLogger::warn(F("Attempt to get rx buffer with %d bytes while buffer has size %d"), tx_rx_buffer_size,
							   k_buffer_size);
			return nullptr;
		} else {
			// reset rx buffer first for cleanness
			memset(_rx_buffer, 0, k_buffer_size);
			return _rx_buffer;
		}
	}

	bool consume(uint8_t *slave_response_buffer, long buffer_size) {
		bool valid = true;
		if (_slave_synchronized) {
			if (buffer_size != k_buffer_size) {
				SerialLogger::error(F("Cannot consume slave output from %s. Buffer size does not match written "
									  "bytes to slave"), k_name);
				valid = false;
			} else {
				valid = consume_commands(slave_response_buffer, buffer_size, _tx_buffer);
			}
		} else {
			if (buffer_size != COMMUNICATION_START_SEQUENCE_LENGTH) {
				SerialLogger::error(F("Cannot consume slave output from %s. Buffer size does not match written bytes "
									  "to slave"), k_name);
				valid = false;
			} else {
				// the latest (n+1) byte send is 0xFF which is needed to read the nth byte
				for (long i = 0; i < COMMUNICATION_START_SEQUENCE_LENGTH - 1; i += 1) {
					const char &rx_byte = slave_response_buffer[i + COMMAND_SPI_RX_OFFSET];
					if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[i]) {
						SerialLogger::trace(F("Slave from %s rx byte on index %d is %x and does match expected byte "
											  "%x "), k_name, i, rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
					} else {
						SerialLogger::warn(F("Slave from %s rx byte on index %d is %x and does not match expected byte "
											 "%x "), k_name, i, rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
						valid = false;
						break;
					}
				}
			}
		}

		if (valid) {
			if (!_slave_synchronized) {
				SerialLogger::info(F("%s slave synchronized with this master"), k_name);
			}
			_slave_synchronized = true;
		} else {
			if (_slave_synchronized) {
				SerialLogger::warn(F("%s slave no longer synchronized with this master"), k_name);
			} else {
				SerialLogger::warn(F("%s slave not synchronized with this master"), k_name);
			}
			_slave_synchronized = false;
		}

		return valid;
	};


	void restart() {
		SerialLogger::info(F("(Re)Starting slave %d (%s) connected to slave-select pin %d with power supply on pin %d"),
						   k_slave_id + 1, k_name, k_slave_pin, k_slave_restart_pin);
		// if we put some voltage on the reset pin, the board will restart
		pinMode(k_slave_restart_pin, OUTPUT);
		digitalWrite(k_slave_restart_pin, HIGH);
		delay(25);
		digitalWrite(k_slave_restart_pin, LOW);
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

	int get_amount_data_push_commands() const {
		return k_amount_data_push_commands;
	};

	int get_amount_data_request_commands() const {
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
	/**
	 * Please note: Passing the data request callbacks with template type allows interpreations of different commands
	 * with different value type if called with different callbacks and an offset to tx, rx buffer leading to maximum
	 * flexibility.
	 */
	template<typename T>
	bool interpret_communication(const uint8_t *tx_buffer, const uint8_t *rx_buffer, const long buffer_size,
								 const int amount_data_request_callbacks,
								 std::vector <std::function<bool(int16_t, T)>> data_request_callbacks) {
		SerialLogger::trace(F("Validating master-slave communication for %s"), k_name);
		uint8_t rxId1[COMMAND_FRAME_ID_SIZE];
		uint8_t rxId2[COMMAND_FRAME_ID_SIZE];
		uint8_t txId1_bytes[COMMAND_FRAME_ID_SIZE];
		uint8_t txId2_bytes[COMMAND_FRAME_ID_SIZE];
		uint8_t rx_value_bytes[COMMAND_FRAME_VALUE_SIZE];
		uint8_t tx_value_bytes[COMMAND_FRAME_VALUE_SIZE];

		for (long i = 0; i + COMMAND_FRAME_SIZE <= buffer_size; i += COMMAND_FRAME_SIZE) {
			// Get the ReqId
			for (int id_counter = 0; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
				//Serial.printf("%d, %d, %d -> %d: %x\n", COMMAND_SPI_RX_OFFSET , i, id_counter, COMMAND_SPI_RX_OFFSET + i + id_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + id_counter]);
				rxId1[id_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + id_counter];
				txId1_bytes[id_counter] = tx_buffer[i + id_counter];
			}

			// Get the command value
			for (int value_counter = 0; value_counter < COMMAND_FRAME_VALUE_SIZE; value_counter++) {
				//Serial.printf("%d: %x\n", COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + value_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + value_counter]);
				rx_value_bytes[value_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE +
														  value_counter];
				tx_value_bytes[value_counter] = tx_buffer[i + COMMAND_FRAME_ID_SIZE + value_counter];
			}

			// Get the AckId == ReqId
			for (int id_counter = 0; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
				//Serial.printf("%d: %x\n", COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter, rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter]);
				rxId2[id_counter] = rx_buffer[COMMAND_SPI_RX_OFFSET + i + COMMAND_FRAME_ID_SIZE +
											  COMMAND_FRAME_VALUE_SIZE +
											  id_counter];
				txId2_bytes[id_counter] = tx_buffer[i + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE + id_counter];
			}

			int16_t txId1 = 0;
			int16_t txId2 = 0;
			memcpy(&txId1, txId1_bytes, sizeof(txId1));
			memcpy(&txId2, txId2_bytes, sizeof(txId2));
			if (txId2 != -1) {
				SerialLogger::warn(F("Master did send wrong 2nd (ack) id which must be -1 but was %d. This is rather "
									 "strange..."), txId2);
			}

			const int16_t id1 = SpiCommands::verifyIds(rxId1, txId1_bytes);
			// txId2 is the request to ack the very first id of a command, thus we use txId1 again to compare against rxId2
			const int16_t id2 = SpiCommands::verifyIds(rxId2, txId1_bytes);
			SerialLogger::trace(F("Comparing %d (txId1) with %d (rxId1) with %d (rxId2/ackId) and value %x%x%x%x"),
								txId1, id1, id2, rx_value_bytes[0], rx_value_bytes[1], rx_value_bytes[2],
								rx_value_bytes[3]);
			if (id1 <= 0 || id2 <= 0) {
				SerialLogger::error(F("Unknown ids received %d, %d"), id1, id2);
				return false;
			} else if (id1 != id2) {
				SerialLogger::error(F("Req and Ack Id do not align %d != %d"), id1, id2);
				return false;
			} else if (id1 > MAX_ID) {
				SerialLogger::warn(F("Received bad id %d > %d (max)"), id1, MAX_ID);
				return false;
			} else {
				T rx_data = 0;
				memcpy(&rx_data, rx_value_bytes, COMMAND_FRAME_VALUE_SIZE);
				bool processed = false;
				for (int i = 0; i < amount_data_request_callbacks && !processed; i++) {
					processed = data_request_callbacks[i](id1, rx_data);
				}

				if (processed) {
					SerialLogger::trace(F("Data response from slave %s is %ld for id %d"), k_name, rx_data, id1);
				} else {
					SerialLogger::trace(F("Data push to slave %s with ack response value %ld for id %d"), k_name,
										rx_data, id1);
					for (int value_counter; value_counter < COMMAND_FRAME_VALUE_SIZE; value_counter++) {
						if (rx_value_bytes[value_counter] != tx_value_bytes[value_counter]) {
							SerialLogger::warn(F("Slave did not return correct value bytes"));
							return false;
						}
					}
				}
			}
		}
		SerialLogger::trace(F("Validating master-slave communication for %s done"), k_name);
		return true;
	}

	virtual void fill_commands_bytes(uint8_t *tx_buffer) = 0;

	virtual bool
	consume_commands(uint8_t *slave_response_buffer, long slave_response_buffer_size, uint8_t *tx_buffer) = 0;

	const int k_amount_data_request_callbacks;

private:
	const int k_slave_id;
	const char *k_name;
	const int k_slave_pin;
	const int k_slave_restart_pin;
	const int k_amount_data_push_commands;
	const long k_buffer_size;

	uint8_t *_tx_buffer;
	uint8_t *_rx_buffer;

	bool _slave_synchronized;
	SpiSlaveHandler *_spi_slave_handler;
};

#endif //LAWNMOVER_UTILS_SPI_SLAVE_H
