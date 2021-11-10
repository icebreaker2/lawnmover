#ifndef ESP32_SPI_MASTER_H
#define ESP32_SPI_MASTER_H

#include <arduino_timer_esp32.h>
#include <esp32-hal-spi.h>
// See https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
#include <SPI.h>
#include "spi_slave_handler.h"
#include "spi_slave_controller.h"

#define MAX_SLAVES 5

class Esp32SpiMaster {
    public:
        Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, void (*error_callback)(),
                       const long frequency = 2000000, const int dma_channel = 1, const uint8_t spi_mode = SPI_MODE0,
                       const int tx_rx_buffer_size = 60, const int chunk_size = 1);
        ~Esp32SpiMaster();

        void addSlave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay, const int interval,
                      const int inter_transaction_delay_microseconds, const long clock_divider, Timer<> &timer,
                      SpiSlaveController *spiSlaveController);

    private:
        const int k_clock_pin;
        const int k_miso_pin;
        const int k_mosi_pin;
        const int k_frequency;
        const int k_dma_channel;
        const uint8_t k_spi_mode;

        void (*error_callback_)();
        volatile bool shutdown_ = false;
        int max_intervall_ = 0;

        const int k_chunk_size;
        const int k_tx_rx_buffer_size;
        uint8_t* spi_master_rx_buf_;

        static SpiSlaveHandler *_slave_handlers[];
        static int free_ids[];
        static int get_free_id();
        static bool add_free_id(const int id);

        SpiSlaveHandler *setup_slave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay,
                                     const int interval, const int slave_id);
        static void restart_slave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay, const int slave_id);
        static void tear_down_slave(const int slave_pin, const int slave_power_pin, const int slave_id);
        static void add_timer(const int slave_id, const int slave_pin, const int slave_power_pin, const int slave_boot_delay,
                              const int interval, const int inter_transaction_delay_microseconds, const long clock_divider,
                              Timer<> &timer, SpiSlaveController *spiSlaveController, const int chunk_size, uint8_t* rx_buffer,
                              int max_tx_rx_buffer_size, void (*error_callback)(), volatile bool &shutdown, SpiSlaveHandler *master);
};

#endif // ESP32_SPI_MASTER_H
