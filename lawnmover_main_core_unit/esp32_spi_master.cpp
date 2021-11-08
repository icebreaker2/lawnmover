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

int Esp32SpiMaster::free_ids[MAX_SLAVES];
ESP32DMASPI::Master *Esp32SpiMaster::masters_[MAX_SLAVES];

Esp32SpiMaster::Esp32SpiMaster(const int clock_pin, const int miso_pin, const int mosi_pin, void (*error_callback)(),
                               const long frequency, const int dma_channel , const int queue_size,
                               const uint8_t spi_mode, const int tx_rx_buffer_size, const int chunk_size) :
    k_clock_pin(clock_pin), k_miso_pin(miso_pin), k_mosi_pin(mosi_pin), k_frequency(frequency),
    k_dma_channel(dma_channel), k_queue_size(queue_size), k_spi_mode(spi_mode),
    k_tx_rx_buffer_size(tx_rx_buffer_size), k_chunk_size(chunk_size) {
    error_callback_ = error_callback;
    // to use DMA buffer, use these methods to allocate buffer
    spi_master_rx_buf_ = (uint8_t*) malloc(k_tx_rx_buffer_size * sizeof spi_master_rx_buf_);
    memset(spi_master_rx_buf_, 0, k_tx_rx_buffer_size);
    delay(1000);

    for (int i = 0; i < MAX_SLAVES; i++) {
        free_ids[i] = i;
    }
}

Esp32SpiMaster::~Esp32SpiMaster() {
    shutdown_ = true;
    delay(max_intervall_);
    for (int i = 0; i < MAX_SLAVES; i++) {
        delete masters_[i];
    }
}

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
void Esp32SpiMaster::addSlave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay,
                              const int interval, const int inter_transaction_delay_microseconds,
                              const long clock_divider, Timer<> &timer, SpiSlaveController *spiSlaveController) {
    const int slave_id = get_free_id();
    if (slave_id >= 0) {
        ESP32DMASPI::Master *master = setup_slave(slave_pin, slave_power_pin, slave_boot_delay, interval, slave_id);

        // For some f***ing reason, we cannot pass this without producing core dumps...
        const int chunk_size = k_chunk_size;
        uint8_t* rx_buffer = spi_master_rx_buf_;
        int max_tx_rx_buffer_size = k_tx_rx_buffer_size;
        void (*error_callback)() = error_callback_;
        volatile bool &shutdown = shutdown_;

        add_timer(slave_id, slave_pin, slave_power_pin, slave_boot_delay, interval, inter_transaction_delay_microseconds,
                  clock_divider, timer, spiSlaveController, chunk_size, rx_buffer, max_tx_rx_buffer_size, error_callback,
                  shutdown, master);
    } else {
        SerialLogger::error("Cannot add a new master to internal array. Reached max of %d masters", MAX_SLAVES);
    }
}


void Esp32SpiMaster::add_timer(const int slave_id, const int slave_pin, const int slave_power_pin, const int slave_boot_delay,
                               const int interval, const int inter_transaction_delay_microseconds, const long clock_divider,
                               Timer<> &timer, SpiSlaveController *spiSlaveController, const int chunk_size, uint8_t* rx_buffer,
                               int max_tx_rx_buffer_size, void (*error_callback)(), volatile bool & shutdown, ESP32DMASPI::Master *master) {
    timer.every(interval, [&shutdown, slave_id, slave_pin, slave_power_pin, slave_boot_delay, interval,
                           inter_transaction_delay_microseconds, clock_divider, timer, chunk_size, max_tx_rx_buffer_size, 
                           spiSlaveController, rx_buffer, error_callback, master](void*) mutable -> bool {
        bool repeat = true;
        if (shutdown) {
            repeat = false;
        } else {
            long tx_rx_buffer_size = -1;
            uint8_t *tx_buffer = spiSlaveController->supply(tx_rx_buffer_size);
            if (tx_rx_buffer_size  < 0) {
                SerialLogger::error("Cannot create spi slave communication. Supplier returned bad buffer_size %d", tx_rx_buffer_size);
                repeat = false;
            } else if (tx_rx_buffer_size  > max_tx_rx_buffer_size) {
                SerialLogger::error("Cannot create spi slave communication. Supplier returned buffer size greater than master can handle. "
                "Requested %d but can only handle %d", tx_rx_buffer_size, max_tx_rx_buffer_size);
                repeat = false;
            } else {
                if (!silly_semaphore_single_threaded && (silly_semaphore_single_threaded = true)) {
                    // reset rx buffer first
                    memset(rx_buffer, 0, tx_rx_buffer_size);
                    for (long counter = 0; counter < tx_rx_buffer_size; counter += chunk_size) {
                        try {
                            size_t sendBytes = master->transfer(tx_buffer + counter,  rx_buffer + counter, chunk_size);

                            // TODO can we omit small delays?
                            delayMicroseconds(inter_transaction_delay_microseconds);

                        } catch (const std::runtime_error &e) {
                            SerialLogger::error("Failed to transfer bytes for slave on slave select %d: %s", slave_pin, e.what());
                            Esp32SpiMaster::tear_down_slave(slave_pin, slave_power_pin, slave_id);
                            repeat = false;
                            break;
                        }
                    }
                    if (repeat) {
                        if (spiSlaveController->consume(rx_buffer, tx_rx_buffer_size)) {
                            SerialLogger::debug("Slave on slave select pin %d did return correct results!", slave_pin);
                        } else {
                            SerialLogger::error("Slave did not return correct results on slave-select pin %d!", slave_pin);
                            restart_slave(slave_pin, slave_power_pin, slave_boot_delay, slave_id);
                        }
                    } else {
                        SerialLogger::error("Spi communication appears broken. Resetting whole spi communication to try best.");
                        (*error_callback)();
                    }
                    silly_semaphore_single_threaded = false;
                } else {
                    SerialLogger::warn("Bus busy. Skipping slave on slave-select pin %d", slave_pin);
                }
            }
        }

        return repeat; // to repeat the action - false to stop
    });
}

int Esp32SpiMaster::get_free_id() {
    int id = -1;
    for (int i = 0; i <  MAX_SLAVES; i++) {
        if (free_ids[i] != -1) {
            id = free_ids[i];
            free_ids[i] = -1;
            break;
        }
    }
    return id;
}

bool Esp32SpiMaster::add_free_id(const int id) {
    bool added = false;
    for (int i = 0; i <  MAX_SLAVES; i++) {
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

ESP32DMASPI::Master *Esp32SpiMaster::setup_slave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay,
        const int interval, const int slave_id) {
    restart_slave(slave_pin, slave_power_pin, slave_boot_delay, slave_id);

    ESP32DMASPI::Master *master = new ESP32DMASPI::Master();
    masters_[slave_id] = master;
    SerialLogger::info("Adding slave %d/%d", slave_id + 1, MAX_SLAVES);
    max_intervall_ = max_intervall_ < interval ? interval : max_intervall_;


    master->setDataMode(k_spi_mode);
    master->setFrequency(k_frequency);
    master->setMaxTransferSize(k_tx_rx_buffer_size);
    // Disabling DMA limits to 64 bytes per transaction only
    master->setDMAChannel(k_dma_channel);  // 1 or 2 only
    master->setQueueSize(k_queue_size);   // transaction queue size
    // VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
    master->begin(VSPI, k_clock_pin, k_miso_pin, k_mosi_pin, slave_pin);
    return master;
}

void Esp32SpiMaster::tear_down_slave(const int slave_pin, const int slave_power_pin, const int slave_id) {
    SerialLogger::info("Shutting down spi communication to slave %d connected to slave-select pin %d with power supply on pin %d",
                       slave_id, slave_pin, slave_power_pin);
    digitalWrite(slave_power_pin, LOW);
    delete masters_[slave_id];
    add_free_id(slave_id);
}

void Esp32SpiMaster::restart_slave(const int slave_pin, const int slave_power_pin, const int slave_boot_delay, const int slave_id) {
    SerialLogger::info("(Re)Starting slave %d connected to slave-select pin %d with power supply on pin %d", slave_id + 1, slave_pin,
                       slave_power_pin);
    pinMode(slave_power_pin, OUTPUT);
    digitalWrite(slave_power_pin, LOW);
    delay(100);

    digitalWrite(slave_power_pin, HIGH);
    // wait some time for the slave to boot
    SerialLogger::info("Powering up slave on slave_select pin %d with power supply on pin %d", slave_pin, slave_power_pin);
    delay(slave_boot_delay);
    SerialLogger::info("Engine Slave needs to synchronize with this master");
}
