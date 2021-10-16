#include <arduino_timer_esp32.h>
#include <esp32-hal-spi.h>
// See https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
#include <SPI.h>
#include "ESP32DMASPIMaster.h"

class Esp32SpiMaster {
    public:
        Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, void (*error_callback)(),
                       const long frequency = 2000000, const int dma_channel = 1, const int queue_size = 1000,
                       const uint8_t spi_mode = SPI_MODE0, const int tx_rx_buffer_size = 60, const int chunk_size = 1);
        ~Esp32SpiMaster();

        void addSlave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay, const int interval,
                      const long clock_divider, Timer<> &timer, uint8_t *(*supplier)(long&), bool(*consumer)(uint8_t *, long));

    private:
        const int k_clock_pin;
        const int k_miso_pin;
        const int k_mosi_pin;
        const int k_frequency;
        const int k_dma_channel;
        const int k_queue_size;
        const uint8_t k_spi_mode;

        void (*error_callback_)();
        volatile bool shutdown_ = false;
        int max_intervall_ = 0;
        ESP32DMASPI::Master *masters_[3];
        int mastersCounter_ = 0;

        const int k_chunk_size;
        const int k_tx_rx_buffer_size;
        uint8_t* spi_master_rx_buf_;


        ESP32DMASPI::Master *setup_slave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay, const int interval);
        static void tear_down_slave(const int slave_pin, const int slave_power_pin);
};
