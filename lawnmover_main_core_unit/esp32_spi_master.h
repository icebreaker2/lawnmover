#ifndef ESP32_SPI_MASTER_H
#define ESP32_SPI_MASTER_H

#include <arduino_timer_esp32.h>
#include <esp32-hal-spi.h>
// See https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
#include <SPI.h>
#include "master_spi_slave.h"

#define MAX_SLAVES 5

class Esp32SpiMaster {
  public:
    static int take_free_id();

    static bool put_free_id(const int id);

    Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, void (*error_callback)(),
                   const long frequency = 2000000, const int dma_channel = 1, const uint8_t spi_mode = SPI_MODE0,
                   const int tx_rx_buffer_size = 60, const int chunk_size = 1, const int inter_transaction_delay_microseconds = 10);
    ~Esp32SpiMaster();

    void put_slave(MasterSpiSlave *spi_slave);

    SpiSlaveHandler *get_handler(const int slave_pin);

    void schedule(const int interval, Timer<> &timer);

  private:

    static void schedule_internal(const int interval, Timer<> &timer, const int chunk_size, uint8_t* rx_buffer, int max_tx_rx_buffer_size, void (*error_callback)(), volatile bool &shutdown,
                                  MasterSpiSlave **slaves, const int inter_transaction_delay_microseconds, volatile int &slave_cursor, const int registered_slaves);

    static MasterSpiSlave *_slaves[];
    static int _registered_slaves;
    static volatile int _slave_cursor;
    static int free_ids[];

    const int k_clock_pin;
    const int k_miso_pin;
    const int k_mosi_pin;
    const int k_frequency;
    const int k_dma_channel;
    const uint8_t k_spi_mode;
    const int k_inter_transaction_delay_microseconds;

    void (*error_callback_)();
    volatile bool shutdown_ = false;

    const int k_chunk_size;
    const int k_tx_rx_buffer_size;
    uint8_t* spi_master_rx_buf_;
};

#endif // ESP32_SPI_MASTER_H
