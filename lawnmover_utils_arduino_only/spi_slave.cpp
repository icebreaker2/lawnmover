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

bool synchronized = false;
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

/**
    SPI interrupt routine
    Note: Printing consumes too much time. Slave does not respond in time.
*/
ISR (SPI_STC_vect) {
		const uint8_t rx_byte = SPDR;
		uint8_t tx_byte = 0;
		const bool previously_synchronized = synchronized;

		const bool full_command_or_synchronized = SpiCommands::slave_process_partial_command(
		synchronized, rx_byte, tx_byte, _data_request_command_callbacks, _amount_data_request_command_callbacks);
		SPDR = tx_byte;
		_rx_buffer[_buffer_counter] = rx_byte;
		_tx_buffer[_buffer_counter] = tx_byte;

		_buffer_counter = (_buffer_counter + 1) % _commands_size;
		if (full_command_or_synchronized && previously_synchronized) {
			const int tx_rx_offset = _buffer_counter == 0 ? _last_commands_size : _buffer_counter - COMMAND_FRAME_SIZE;
			const bool command_interpreted = SpiCommands::slave_interpret_command(_rx_buffer + tx_rx_offset,
																				  _data_push_command_callbacks,
																				  _amount_data_push_command_callbacks);
			if (!command_interpreted) {
				// Logging (serial printing is faster) must be kept to an absolute minimum for this SPI routine depending on the logging baudrate.
				SerialLogger::warn("Did not receive valid command. Cannot interpret value.");
			}
		} else if (!previously_synchronized && synchronized) {
			// We are now synchronized and need to start from index 0 for everything_
			_buffer_counter = 0;
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
		if (synchronized) {
			SerialLogger::info("Slave is synchronized");
		} else {
			SerialLogger::info("Slave is NOT synchronized");
		}

		return true; // to repeat the action - false to stop
	});
}
