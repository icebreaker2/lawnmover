#include "spi_slave.h"

#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>
#include <spi_commands.h>


int _commands_size;
int _last_commands_size;
int _buffer_counter = 0;
uint8_t *_rx_buffer;
uint8_t *_tx_buffer;

bool _synchronized = false;
int _amount_data_push_command_callbacks;
bool (**_data_push_command_callbacks)(int16_t, int16_t);
int _amount_data_request_command_callbacks;
bool (**_data_request_command_callbacks)(int16_t, uint8_t *);

SpiSlave::SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
				   bool (*data_push_command_callbacks[])(int16_t, int16_t),
				   const int amount_data_push_command_callbacks,
				   bool (*data_request_command_callbacks[])(int16_t, uint8_t *),
				   const int amount_data_request_command_callbacks, const int buffer_length) {
	pinMode(sck_pin, INPUT);
	pinMode(mosi_pin, INPUT);
	pinMode(miso_pin, OUTPUT);
	pinMode(ss_pin, INPUT);

	/*  Setup SPI control register SPCR
		SPIE - Enables the SPI interrupt when 1
		SPE - Enables the SPI when 1
		DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
		MSTR - Sets the Arduino in master mode when 1, slave mode when 0
		CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
		CPHA - Samples data on the trailing edge of the data clock when 1, leading edge when 0
		SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)
	*/
	// enable SPI subsystem and set correct SPI mode
	// SPCR = (1<<SPE)|(0<<DORD)|(0<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(1<<SPR0);

	// SPI status register: SPSR
	// SPI data register: SPDR
	// turn on SPI in slave mode
	SPCR |= bit(SPE);
	// turn on interrupts
	SPCR |= bit(SPIE);

	_data_push_command_callbacks = data_push_command_callbacks;
	_amount_data_push_command_callbacks = amount_data_push_command_callbacks;
	_data_request_command_callbacks = data_request_command_callbacks;
	_amount_data_request_command_callbacks = amount_data_request_command_callbacks;

	_commands_size = buffer_length;
	_last_commands_size = _commands_size - COMMAND_FRAME_SIZE;
	_rx_buffer = (uint8_t *) calloc(_commands_size, sizeof *_rx_buffer);
	_tx_buffer = (uint8_t *) calloc(_commands_size, sizeof *_tx_buffer);
}

int _current_command_cursor = 0;
uint8_t _current_command_id_bytes[COMMAND_FRAME_ID_SIZE];
int16_t _current_command_id = -1;
uint8_t _current_command_value_bytes[COMMAND_FRAME_VALUE_SIZE];

void synchronize(const uint8_t rx_byte, uint8_t &tx_byte) {
	if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor]) {
		if (_current_command_cursor == COMMUNICATION_START_SEQUENCE_LENGTH - 1) {
			_current_command_cursor = 0;
			_current_command_id = -1;
			tx_byte = 0;
			if (rx_byte == 0xFF) {
				_synchronized = true;
				return;
			} else {
				SerialLogger::warn("Bad end-of-sequence-byte received (%x) which should have been %x", rx_byte,
								   SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor]);
			}
		} else {
			tx_byte = SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor];
			_current_command_cursor++;
		}
	} else {
		_current_command_cursor = 0;
		_current_command_id = -1;
		//SerialLogger::warn("Lost synchronization. Received byte %c but expected %c at index %d. Restarting synchronization sequence.", rx_byte, SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor], _current_command_cursor);
	}
	_synchronized = false;
}

bool process_partial_command(const uint8_t rx_byte, uint8_t &tx_byte) {
	if (_current_command_cursor < COMMAND_FRAME_ID_SIZE) {
		_current_command_id_bytes[_current_command_cursor] = rx_byte;
		_current_command_cursor++;
		tx_byte = rx_byte;
	} else if (_current_command_cursor < COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE) {
		uint8_t write_byte = rx_byte;
		if (_current_command_cursor == COMMAND_FRAME_ID_SIZE) {
			memcpy(&_current_command_id, _current_command_id_bytes, sizeof(int16_t));
			bool data_request = false;
			for (int i = 0; i < _amount_data_request_command_callbacks && !data_request; i++) {
				data_request = (*_data_request_command_callbacks[i])(_current_command_id, _current_command_value_bytes);
			}
			if (!data_request) {
				_current_command_id = -1;
			}
		}
		if (_current_command_id > 0) {
			// This is a data request command and a callback filled the value buffer
			write_byte = _current_command_value_bytes[_current_command_cursor - COMMAND_FRAME_ID_SIZE];
		} else {
			// This is a data push command and a callback will need the value bytes
			_current_command_value_bytes[_current_command_cursor - COMMAND_FRAME_ID_SIZE] = rx_byte;
		}
		tx_byte = write_byte;
		_current_command_cursor++;
	} else if (_current_command_cursor < COMMAND_FRAME_SIZE - COMMAND_SPI_RX_OFFSET) {
		const int &id_index = _current_command_cursor - (COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE);
		if (rx_byte == 0xFF) {
			tx_byte = _current_command_id_bytes[id_index];
			_current_command_cursor++;
		} else {
			//SerialLogger::warn("Received bad ack id request with byte %c on id index %d with position %d. Synchronization lost.", rx_byte, id_index, _current_command_cursor);
			tx_byte = 0;
			_synchronized = false;
			_current_command_cursor = 0;
			_current_command_id = -1;
		}
	} else {
		// For the master to receive the nth byte we need to send a n+1 byte
		tx_byte = 0;
		_current_command_cursor = 0;
		_current_command_id = -1;
		return true;
	}
	return false;
}

bool post_process_spi_interrupt_routine(const uint8_t rx_byte, const uint8_t tx_byte) {
	SPDR = tx_byte;
	_rx_buffer[_buffer_counter] = rx_byte;
	_tx_buffer[_buffer_counter] = tx_byte;
	_buffer_counter = (_buffer_counter + 1) % _commands_size;
	return true;
}

/**
    SPI interrupt routine
    Note: Printing consumes too much time. Slave does not respond in time.
*/
ISR (SPI_STC_vect) {
		const uint8_t rx_byte = SPDR;
		uint8_t tx_byte = 0;

		if (_synchronized) {
			if (process_partial_command(rx_byte, tx_byte) &
				post_process_spi_interrupt_routine(rx_byte, tx_byte)) {
				const int tx_rx_offset =
						_buffer_counter == 0 ? _last_commands_size : _buffer_counter - COMMAND_FRAME_SIZE;
				const bool command_interpreted = SpiCommands::slave_interpret_command(_rx_buffer + tx_rx_offset,
																					  _data_push_command_callbacks,
																					  _amount_data_push_command_callbacks);
				if (!command_interpreted) {
					// Logging (serial printing is faster) must be kept to an absolute minimum for this SPI routine depending on the logging baudrate.
					SerialLogger::warn("Did not receive valid command. Cannot interpret value.");
					_synchronized = false;
				}
			}
		} else {
			synchronize(rx_byte, tx_byte);
			post_process_spi_interrupt_routine(rx_byte, tx_byte);
			if (_synchronized) {
				// We are now synchronized and need to start from index 0 for everything_
				_buffer_counter = 0;
			}
		}
}  // end of interrupt service routine (ISR) SPI_STC_vect

void SpiSlave::addSlavePrinting(Timer<> &timer, const int interval) {
	timer.every(interval, [](void *) -> bool {
		Serial.print("RxBufferInput:");
		for (long i = 0; i < _commands_size; i += 1) {
			if (i % COMMAND_FRAME_SIZE == 0) {
				Serial.print(" ");
			}
			Serial.print(_rx_buffer[i], HEX);
		}
		Serial.println();


		Serial.print("TxBufferInput:");
		for (long i = 0; i < _commands_size; i += 1) {
			if (i % COMMAND_FRAME_SIZE == 0) {
				Serial.print(" ");
			}
			Serial.print(_tx_buffer[i], HEX);
		}
		Serial.println();
		if (_synchronized) {
			SerialLogger::info("Slave is synchronized");
		} else {
			SerialLogger::info("Slave is NOT synchronized");
		}

		return true; // to repeat the action - false to stop
	});
}
