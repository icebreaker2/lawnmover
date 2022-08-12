#ifndef SPI_SLAVE_H
#define SPI_SLAVE_H

#include <arduino_timer_uno.h>

class SpiSlave {
public:
	SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
			 bool (*commands[])(int16_t, int16_t), const int amount_commands, const int buffer_length);

	~SpiSlave() {
		// nothing to do...
	};

	void addSlavePrinting(Timer<> &timer, const int interval);

private:
};

#endif // SPI_SLAVE_H
