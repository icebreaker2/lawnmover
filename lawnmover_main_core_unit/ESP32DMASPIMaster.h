/**
    MIT License

    Original Copyright (c) 2019 Hideaki Tai
    Modifications Copyright (c) 2021 Hendrik Kahlen

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once
#ifndef ESP32DMASPI_MASTER_H
#define ESP32DMASPI_MASTER_H

#include <Arduino.h>
#include <SPI.h>
#include <driver/spi_master.h>
#include <deque>
#include "spi_device.h"

namespace arduino {
namespace esp32 {
namespace spi {
namespace dma {

class Master {
        static spi_bus_config_t _bus_cfg;
        static bool _initialized_once;
        static spi_host_device_t _host ;
        static uint8_t _mode;
        static uint32_t _frequency;
        static int _queue_size;

        static int _dma_chan;
        static int _max_size;
        static std::deque<spi_transaction_t> _transactions;

        spi_device_interface_config_t _if_cfg;
        spi_device_handle_t _handle;

    public:

        bool init_bus(const int8_t sck, const int8_t miso, const int8_t mosi, const uint8_t spi_bus);
        bool begin(const int8_t sck, const int8_t miso, const int8_t mosi, const int8_t ss, const uint8_t spi_bus = HSPI);
        bool end();

        uint8_t* allocDMABuffer(const size_t s);

        // execute transaction and wait for transmission one by one
        size_t transfer(const uint8_t* tx_buf, const size_t size);
        size_t transfer(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size);

        // queueing transaction and execute simultaneously
        // wait (blocking) and timeout occurs if queue is full with transaction
        // (but designed not to queue transaction more than queue_size, so there is no timeout argument)
        bool queue(const uint8_t* tx_buf, const size_t size);
        bool queue(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size);
        void yield();

        // set these optional parameters before begin() if you want
        void setDataMode(const uint8_t mode);
        void setFrequency(const uint32_t frequency);
        void setMaxTransferSize(const int max_size);
        void setDMAChannel(const int channel);
        void setQueueSize(const int queue_size);

    private:
        void addTransaction(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size);

        SpiDevice *_spi_device;
};

}  // namespace dma
}      // namespace spi
}  // namespace esp32
}  // namespace arduino

#ifndef ARDUINO_ESP32_DMA_SPI_NAMESPACE_BEGIN
#define ARDUINO_ESP32_DMA_SPI_NAMESPACE_BEGIN \
    namespace arduino {                       \
    namespace esp32 {                     \
    namespace spi {                   \
    namespace dma {
#endif
#ifndef ARDUINO_ESP32_DMA_SPI_NAMESPACE_END
#define ARDUINO_ESP32_DMA_SPI_NAMESPACE_END \
    }                                       \
    }                                       \
    }                                       \
    }
#endif

namespace ESP32DMASPI = arduino::esp32::spi::dma;

#endif  // ESP32DMASPI_MASTER_H
