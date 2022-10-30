#include "spi_slave.h"

#include <SPI.h>
#include <Arduino.h>

#include <serial_logger.h>
#include <spi_commands.h>

int _commands_size;
int _buffer_counter = 0;
uint8_t *_rx_buffer;
uint8_t *_tx_buffer;

bool _synchronized = false;
int _amount_data_push_command_callbacks;
bool (**_data_push_command_callbacks)(int16_t, int16_t);
int _amount_data_request_command_callbacks;

bool (**_data_request_command_callbacks)(int16_t, uint8_t *);

void SpiSlave::ISRfromArgs(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
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
	_rx_buffer = (uint8_t *) calloc(_commands_size, sizeof *_rx_buffer);
	_tx_buffer = (uint8_t *) calloc(_commands_size, sizeof *_tx_buffer);
}

int _current_command_cursor = 0;
uint8_t _current_command_id_bytes[COMMAND_FRAME_ID_SIZE];
int16_t _current_command_id = -1;
uint8_t _current_command_value_bytes[COMMAND_FRAME_VALUE_SIZE];
bool _current_command_data_request = false;

void synchronize(const uint8_t rx_byte, uint8_t &tx_byte) {
	if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor]) {
		if (_current_command_cursor == COMMUNICATION_START_SEQUENCE_LENGTH - 1) {
			_current_command_cursor = 0;
			tx_byte = 0;
			if (rx_byte == 0xFF) {
				_synchronized = true;
				return;
			} else {
				SerialLogger::trace(F("Bad end-of-sequence-byte was %x != %x"), rx_byte,
									SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor]);
			}
		} else {
			tx_byte = SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor];
			_current_command_cursor++;
		}
	} else {
		_current_command_cursor = 0;
		SerialLogger::trace(F("Bad sync-sequence-byte was %x != %x"), rx_byte,
							SpiCommands::COMMUNICATION_START_SEQUENCE[_current_command_cursor]);
	}
	_synchronized = false;
}

bool check_and_set_id() {
	memcpy(&_current_command_id, _current_command_id_bytes, sizeof(_current_command_id));
	if (_current_command_id <= 0) {
		SerialLogger::error(F("Bad Id Received. %d is unknown"), _current_command_id);
		return false;
	} else if (_current_command_id > MAX_ID) {
		SerialLogger::warn(F("Received bad id %d > %d (max)"), _current_command_id, MAX_ID);
		return false;
	} else {
		return true;
	}
}

bool process_partial_command(const uint8_t rx_byte, uint8_t &tx_byte) {
	if (_current_command_cursor < COMMAND_FRAME_ID_SIZE) {
		if (_current_command_cursor == 0) {
			// new command; new id but we need to set it at the beginning or the id cannot be used at interpretation
			_current_command_id = -1;
			_current_command_data_request = false;
		}
		_current_command_id_bytes[_current_command_cursor] = rx_byte;
		_current_command_cursor++;
		tx_byte = rx_byte;
	} else if (_current_command_cursor < COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE) {
		uint8_t write_byte = rx_byte;
		if (_current_command_cursor == COMMAND_FRAME_ID_SIZE) {
			// check&set the id and reset it if it fails
			if (!check_and_set_id()) {
				// Bad Req Id received; failure.
				tx_byte = 0;
				_synchronized = false;
				_current_command_cursor = 0;
				return false;
			}
			// inspect for data request commands
			for (int i = 0; i < _amount_data_request_command_callbacks && !_current_command_data_request; i++) {
				_current_command_data_request = (*_data_request_command_callbacks[i])(_current_command_id,
																					  _current_command_value_bytes);
			}
		}
		if (_current_command_data_request) {
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
			// Bad Ack Id byte received; failure.
			tx_byte = 0;
			_synchronized = false;
			_current_command_cursor = 0;
		}
	} else {
		// For the master to receive the nth byte we need to send a n+1 byte
		tx_byte = 0;
		_current_command_cursor = 0;
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

/*
    Call if value received by master
*/
template<typename V>
void interpret_data_push_command(const int id, uint8_t *value_bytes, bool (*data_push_callbacks[])(int16_t, V),
								 const int amount_data_push_callbacks) {
	bool valid = false;
	V value;
	memcpy(&value, value_bytes, sizeof(value));
	for (int i = 0; i < amount_data_push_callbacks && !valid; i++) {
		valid = (*data_push_callbacks[i])(id, value);
	}

	if (!valid) {
		// Logging (serial printing is faster) must be kept to an absolute minimum for this SPI routine depending on the logging baudrate.
		SerialLogger::warn(F("Did not receive valid data push command. Cannot interpret value."));
		_synchronized = false;
	}
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
				if (!_current_command_data_request) {
					interpret_data_push_command(_current_command_id, _current_command_value_bytes,
												_data_push_command_callbacks, _amount_data_push_command_callbacks);
				}
			}
		} else {
			synchronize(rx_byte, tx_byte);
			post_process_spi_interrupt_routine(rx_byte, tx_byte);
			_buffer_counter = _current_command_cursor;
		}
}  // end of interrupt service routine (ISR) SPI_STC_vect

void SpiSlave::addDebugSlavePrinting(Timer<> &timer, const int interval) {
	timer.every(interval, [](void *) -> bool {
		if (SerialLogger::isBelow(SerialLogger::LOG_LEVEL::DEBUG) || !_synchronized) {
			Serial.print(F("RxBufferInput:"));
			for (long i = 0; i < _commands_size; i += 1) {
				if (i % COMMAND_FRAME_SIZE == 0) {
					Serial.print(F(" "));
				}
				Serial.print(_rx_buffer[i], HEX);
			}
			Serial.println();

			Serial.print(F("TxBufferInput:"));
			for (long i = 0; i < _commands_size; i += 1) {
				if (i % COMMAND_FRAME_SIZE == 0) {
					Serial.print(F(" "));
				}
				Serial.print(_tx_buffer[i], HEX);
			}
			Serial.println();
			if (_synchronized) {
				SerialLogger::info(F("Slave is synchronized"));
			} else {
				SerialLogger::info(F("Slave is NOT synchronized"));
			}
		}
		return true; // to repeat the action - false to stop
	});
}
