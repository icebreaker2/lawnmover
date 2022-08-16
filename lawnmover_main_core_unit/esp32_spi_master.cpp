#include "esp32_spi_master.h"

#include <Arduino.h>
#include <serial_logger.h>

// See: https://forum.arduino.cc/t/esp32-spi-comunication-on-sd-module/598345
/*  The default pins for SPI on the ESP32.
    HSPI
    MOSI = GPIO13
    MISO = GPIO12
    CLK = GPIO14
    CD = GPIO15

    VSPI
    MOSI = GPIO23
    MISO = GPIO19
    CLK/SCK = GPIO18
    CS/SS = GPIO5
*/

// ISO C++ forbids in-class initialization of non-const static members
int Esp32SpiMaster::free_ids[MAX_SLAVES];
MasterSpiSlave *Esp32SpiMaster::_slaves[MAX_SLAVES];
int Esp32SpiMaster::_registered_slaves = 0;
volatile int Esp32SpiMaster::_slave_cursor = 0;

Esp32SpiMaster::Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, void (*error_callback)(),
							   const long frequency, const int dma_channel, const uint8_t spi_mode,
							   const int tx_rx_buffer_size, const int chunk_size,
							   const int inter_transaction_delay_microseconds) :
		k_clock_pin(clock_pin), k_miso_pin(miso_pin), k_mosi_pin(mosi_pin), k_frequency(frequency),
		k_dma_channel(dma_channel), k_spi_mode(spi_mode), k_tx_rx_buffer_size(tx_rx_buffer_size),
		k_chunk_size(chunk_size), k_inter_transaction_delay_microseconds(inter_transaction_delay_microseconds) {
	error_callback_ = error_callback;

	// to use DMA buffer, use these methods to allocate buffer
	spi_master_rx_buf_ = (uint8_t *) malloc(k_tx_rx_buffer_size * sizeof spi_master_rx_buf_);
	memset(spi_master_rx_buf_, 0, k_tx_rx_buffer_size);
	delay(1000);

	for (int i = 0; i < MAX_SLAVES; i++) {
		free_ids[i] = i;
	}
}

Esp32SpiMaster::~Esp32SpiMaster() {
	SerialLogger::info("Shutting down all slaves");
	shutdown_ = true;
	for (int i = 0; i < _registered_slaves; i++) {
		put_free_id(_slaves[i]->get_slave_id());
		delete _slaves[i];
	}
}

/**
    Method to add slave communication to timer given the intervall, where suppliers yield the
    commands to send and consumer consumes the results read from slave. Consumer must respond
    with if received values were valid (true) or not (false).

    Note: std::function use was not possible or we will face weird core dumps if we call with more than
    one std::function -- even if function gets never called. If we pass one std::function, everything works.
    Thus, downgraded to C-like function pointers.
*/
void Esp32SpiMaster::put_slave(MasterSpiSlave *spi_slave) {
	if (spi_slave == nullptr) {
		SerialLogger::error("Cannot add new slave to internal array. The spi Slave is a nullptr");
	} else {
		_slaves[_registered_slaves] = spi_slave;
		SerialLogger::info("Adding slave %d/%d", spi_slave->get_slave_id() + 1, MAX_SLAVES);
		_registered_slaves++;
	}
}


void Esp32SpiMaster::schedule(const int interval, Timer<> &timer) {
	// For some f***ing reason, we cannot pass this without producing core dumps...
	const int chunk_size = k_chunk_size;
	uint8_t *rx_buffer = spi_master_rx_buf_;
	int max_tx_rx_buffer_size = k_tx_rx_buffer_size;
	void (*error_callback)() = error_callback_;
	volatile bool &shutdown = shutdown_;
	MasterSpiSlave **slaves = _slaves;
	const int inter_transaction_delay_microseconds = k_inter_transaction_delay_microseconds;
	volatile int &slave_cursor = _slave_cursor;
	const int registered_slaves = _registered_slaves;

	schedule_internal(interval, timer, chunk_size, rx_buffer, max_tx_rx_buffer_size, error_callback, shutdown, slaves,
					  inter_transaction_delay_microseconds, slave_cursor, registered_slaves);
}

void Esp32SpiMaster::schedule_internal(const int interval, Timer<> &timer, const int chunk_size, uint8_t *rx_buffer,
									   int max_tx_rx_buffer_size, void (*error_callback)(), volatile bool &shutdown,
									   MasterSpiSlave **slaves, const int inter_transaction_delay_microseconds,
									   volatile int &slave_cursor, const int registered_slaves) {
	timer.every(interval,
				[&shutdown, &slave_cursor, inter_transaction_delay_microseconds, chunk_size, max_tx_rx_buffer_size,
						rx_buffer, error_callback, slaves, registered_slaves](void *) mutable -> bool {
					bool repeat = true;
					if (shutdown) {
						repeat = false;
					} else if (registered_slaves < 1) {
						SerialLogger::error(
								"No slaves registered. No need to start a timer for spi communication for no spi slaves");
						return false;
					} else {
						MasterSpiSlave *spi_slave = slaves[slave_cursor];
						slave_cursor = (slave_cursor + 1) % registered_slaves;
						SerialLogger::debug("Initiating spi communication with slave %d (%s) and moved cursor to %d.",
											spi_slave->get_slave_id(), spi_slave->get_name(), slave_cursor);

						long tx_rx_buffer_size = -1;
						uint8_t *tx_buffer = spi_slave->supply(tx_rx_buffer_size);
						if (tx_rx_buffer_size < 0) {
							SerialLogger::error(
									"Cannot create spi slave communication. Supplier returned bad buffer_size %d",
									tx_rx_buffer_size);
							repeat = false;
						} else if (tx_rx_buffer_size > max_tx_rx_buffer_size) {
							SerialLogger::error("Cannot create spi slave communication. Supplier returned buffer size "
												"greater than master can handle. Requested %d but can only handle %d",
												tx_rx_buffer_size, max_tx_rx_buffer_size);
							repeat = false;
						} else {
							// reset rx buffer first
							// TODO offset buffer or replace with buffer from slave
							memset(rx_buffer, 0, tx_rx_buffer_size);
							for (long counter = 0; counter < tx_rx_buffer_size; counter += chunk_size) {
								try {
									size_t sendBytes = spi_slave->get_spi_slave_handler()->transfer(tx_buffer + counter, 
									                                                                rx_buffer + counter,
									                                                                chunk_size);

									// TODO can we omit small delays?
									delayMicroseconds(inter_transaction_delay_microseconds);
								} catch (const std::runtime_error &e) {
									SerialLogger::error("Failed to transfer bytes for slave on slave select %d: %s",
														spi_slave->get_slave_pin(), e.what());
									spi_slave->restart();
									repeat = false;
									break;
								}
							}
							if (repeat) {
								if (spi_slave->consume(rx_buffer, tx_rx_buffer_size)) {
									SerialLogger::debug("Slave on slave select pin %d (%s) did return correct results!",
														spi_slave->get_slave_pin(), spi_slave->get_name());
								} else {
									SerialLogger::error(
											"Slave on slave select pin %d (%s) did NOT return correct results!",
											spi_slave->get_slave_pin(), spi_slave->get_name());
									spi_slave->restart();
								}
							} else {
								SerialLogger::error(
										"Spi communication appears broken. Resetting whole spi communication to try best.");
								(*error_callback)();
							}
						}

                        if (SerialLogger::isBelow(SerialLogger::TRACE)) {
                            Serial.printf("RxBufferInput (Slave %d: %s): ", spi_slave->get_slave_id() + 1, spi_slave->get_name());
                            for (long i = 0; i < tx_rx_buffer_size; i += 1) {
                                if (i % COMMAND_FRAME_SIZE == 0) {
                                    Serial.print(" ");
                                }
                                Serial.print(rx_buffer[i], HEX);
                            }
                            Serial.println();
                            Serial.printf("TxBufferInput (Slave %d: %s):", spi_slave->get_slave_id() + 1, spi_slave->get_name());
                            for (long i = 0; i < tx_rx_buffer_size; i += 1) {
                                if (i % COMMAND_FRAME_SIZE == 0) {
                                    Serial.print(" ");
                                }
                                Serial.print(tx_buffer[i], HEX);
                            }
                            Serial.println();  
                        }
					}
					return repeat; // to repeat the action - false to stop
				});
}

int Esp32SpiMaster::take_free_id() {
	int id = -1;
	for (int i = 0; i < MAX_SLAVES; i++) {
		if (free_ids[i] != -1) {
			id = free_ids[i];
			free_ids[i] = -1;
			break;
		}
	}
	return id;
}

bool Esp32SpiMaster::put_free_id(const int id) {
	bool added = false;
	for (int i = 0; i < MAX_SLAVES; i++) {
		if (free_ids[i] == -1) {
			free_ids[i] = id;
			added = true;
		}
	}
	if (!added) {
		SerialLogger::warn("Could not add free Id %d back to repository.", id);
	}
	return added;
}

SpiSlaveHandler *Esp32SpiMaster::get_handler(const int slave_pin) {
	SpiSlaveHandler *slave_handler = new SpiSlaveHandler();
	slave_handler->setDataMode(k_spi_mode);
	slave_handler->setFrequency(k_frequency);
	slave_handler->setMaxTransferSize(k_tx_rx_buffer_size);
	// Disabling DMA limits to 64 bytes per transaction only
	slave_handler->setDMAChannel(k_dma_channel);  // 1 or 2 only
	// VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
	slave_handler->begin(k_clock_pin, k_miso_pin, k_mosi_pin, slave_pin, VSPI);
	return slave_handler;
}
