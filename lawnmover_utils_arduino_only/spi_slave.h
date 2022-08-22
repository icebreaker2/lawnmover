#ifndef SPI_SLAVE_H
#define SPI_SLAVE_H

#include <arduino_timer_uno.h>

class SpiSlave {
public:
	static void ISRfromArgs(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
							bool (*data_push_command_callbacks[])(int16_t, int16_t),
							const int amount_data_push_command_callbacks,
							bool (*data_request_command_callbacks[])(int16_t, uint8_t *),
							const int amount_data_request_command_callbacks, const int buffer_length);

	static void addDebugSlavePrinting(Timer<> &timer, const int interval);

};

#endif // SPI_SLAVE_H
