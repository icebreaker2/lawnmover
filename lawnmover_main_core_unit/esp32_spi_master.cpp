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

Esp32SpiMaster::Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, const long frequency,
                               const int dma_channel , const int queue_size, const uint8_t spi_mode,
                               const int tx_rx_buffer_size, const int chunk_size) :
    k_clock_pin(clock_pin), k_miso_pin(miso_pin), k_mosi_pin(mosi_pin), k_frequency(frequency),
    k_dma_channel(dma_channel), k_queue_size(queue_size), k_spi_mode(spi_mode),
    k_tx_rx_buffer_size(tx_rx_buffer_size), k_chunk_size(chunk_size) {
    // to use DMA buffer, use these methods to allocate buffer
    spi_master_rx_buf_ = (uint8_t*) heap_caps_malloc(k_tx_rx_buffer_size, MALLOC_CAP_DMA);
    memset(spi_master_rx_buf_, 0, k_tx_rx_buffer_size);
    delay(1000);
}

Esp32SpiMaster::~Esp32SpiMaster() {
    for (int i = 0; i < sizeof(masters_) / sizeof(masters_[0]); i++) {
        delete masters_[i];
    }
}

long counter = 0;
// TODO fix with semaphore
volatile boolean silly_semaphore_single_threaded = false;
// Reference: https://rabbit-note.com/2019/01/20/esp32-arduino-spi-slave/
/**
    Method to add slave communication to timer given the intervall, where suppliers yield the
    commands to send and consumer consumes the results read from slave. Consumer must respond
    with if received values were valid (true) or not (false).

    Note: std::function use was not possible or we will face weird core dumps if we call with more than
    one std::function -- even if function gets never called. If we pass one std::function, everything works.
    Thus, downgraded to C-like function pointers.
*/
void Esp32SpiMaster::addSlave(const int slave_pin, const int interval, const long clock_divider, Timer<> &timer,
                              uint8_t *(*supplier)(long&), bool(*consumer)(uint8_t *, long)) {
    ESP32DMASPI::Master *master = new ESP32DMASPI::Master();
    if (mastersCounter_ < sizeof(masters_) / sizeof(masters_[0])) {
        Serial.println(mastersCounter_);
        masters_[mastersCounter_++] = master;
        master->setDataMode(k_spi_mode);
        master->setFrequency(k_frequency);
        master->setMaxTransferSize(k_tx_rx_buffer_size);
        // Disabling DMA limits to 64 bytes per transaction only
        master->setDMAChannel(k_dma_channel);  // 1 or 2 only
        master->setQueueSize(k_queue_size);   // transaction queue size
        // VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
        master->begin(VSPI, k_clock_pin, k_miso_pin, k_mosi_pin, slave_pin);

        // For some f***ing reason, we cannot pass this without producing core dumps....
        const int chunk_size = k_chunk_size;
        uint8_t* rx_buffer = spi_master_rx_buf_;
        long tx_rx_buffer_size = -1;
        uint8_t *tx_buffer = (*supplier)(tx_rx_buffer_size);
        SerialLogger::debug("Setting up tx buffer with size %d", tx_rx_buffer_size);
        if (tx_rx_buffer_size  < 0) {
            SerialLogger::error("Cannot create spi slave communication. Supplier returned bad buffer_size %d", tx_rx_buffer_size);
        } else if (tx_rx_buffer_size  > k_tx_rx_buffer_size ) {
            SerialLogger::error("Cannot create spi slave communication. Supplier returned buffer size greater than master can handle. Requested %d but can only handle %d", tx_rx_buffer_size, k_tx_rx_buffer_size);
        } else {
            timer.every(interval, [slave_pin, chunk_size, tx_rx_buffer_size, rx_buffer, tx_buffer, master, consumer](void*) -> bool {
                if (!silly_semaphore_single_threaded && (silly_semaphore_single_threaded = true)) {
                    // TODO implement
                    // set buffer data here
                    // TODO set buffer
                    // start and wait to complete transaction
                    uint8_t *rxPointer;
                    uint8_t *txPointer;
                    if (counter < tx_rx_buffer_size) {
                        SerialLogger::trace("Increasing pointer");
                        rxPointer = rx_buffer + counter;
                        txPointer = tx_buffer + counter;
                    } else {
                        SerialLogger::trace("Resetting pointer");
                        rxPointer = rx_buffer;
                        txPointer  = tx_buffer;
                        counter = 0;
                        if ((*consumer)(rx_buffer, tx_rx_buffer_size)) {
                            SerialLogger::debug("Slave on slave select pin %d did return correct results!", slave_pin);                            
                        } else {
                            SerialLogger::error("Slave did not return correct results on slave-select pin %d!", slave_pin);
                            return false;
                        }
                    }

                    Serial.printf("Transfering %d (counter: %d)\n", txPointer[0], counter);
                    size_t sendBytes = master->transfer(txPointer, rxPointer, chunk_size);
                    Serial.printf("Received %d bytes\n", sendBytes);
                    counter += chunk_size;
                    silly_semaphore_single_threaded = false;
                } else {
                    SerialLogger::warn("Bus busy. Skipping slave on slave-select pin %d", slave_pin);
                }
                return true; // to repeat the action - false to stop
            });
        }
    } else {
        SerialLogger::error("Cannot add a new master to internal array. Reached max of %d masters", mastersCounter_);
    }
}
