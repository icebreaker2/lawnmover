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

#include "ESP32DMASPIMaster.h"
#include <sstream>
#include "serial_logger.h"

ARDUINO_ESP32_DMA_SPI_NAMESPACE_BEGIN

spi_bus_config_t Master::_bus_cfg;
bool Master::_initialized_once = false;
spi_host_device_t Master::_host  = HSPI_HOST;
uint8_t Master::_mode = SPI_MODE3;
uint32_t Master::_frequency = SPI_MASTER_FREQ_8M;
int Master::_queue_size = 1;
int Master::_dma_chan = 0;     // must be 1 or 2 or 0 if deactivated (limits transaction size to 64 bytes)
int Master::_max_size = 4094;  // default size
std::deque<spi_transaction_t> Master::_transactions; // Default init with constructor

bool Master::init_bus(const int8_t sck, const int8_t miso, const int8_t mosi, const uint8_t spi_bus) {
    if (_initialized_once) {
        SerialLogger::info("Not initializing SPI bus again. Already initialized.");
    } else {
        _bus_cfg.sclk_io_num = sck;
        _bus_cfg.miso_io_num = miso;
        _bus_cfg.mosi_io_num = mosi;
        _bus_cfg.max_transfer_sz = _max_size;

        // make sure to use DMA buffer
        if (_dma_chan == 0) {
            printf("Using no DMA");
        } else if ((_dma_chan != 1) && (_dma_chan != 2)) {
            printf("[WARNING] invalid DMA channel %d, force to set channel 1. make sure to select 1 or 2\n", _dma_chan);
            _dma_chan = 1;
        }

        _host = (spi_bus == HSPI) ? HSPI_HOST : VSPI_HOST;
        esp_err_t e = spi_bus_initialize(_host, &_bus_cfg, _dma_chan);
        if (e == ESP_OK) {
            _initialized_once = true;
            SerialLogger::info("SPI bus initialize succeeded.");
        } else {
            printf("[ERROR] SPI bus initialize failed : %d\n", e);
        }
    }
    return _initialized_once;
}

bool Master::begin(const int8_t sck, const int8_t miso, const int8_t mosi, const int8_t ss, const uint8_t spi_bus) {
    bool valid = init_bus(sck, miso, mosi, spi_bus);

    _if_cfg.mode = _mode;
    _if_cfg.clock_speed_hz = _frequency;
    _if_cfg.queue_size = _queue_size;
    _if_cfg.flags = SPI_DEVICE_NO_DUMMY;
    // TODO: callback??
    _if_cfg.pre_cb = NULL;
    _if_cfg.post_cb = NULL;

    _if_cfg.spics_io_num = ss;

    esp_err_t e = spi_bus_add_device(_host, &_if_cfg, &_handle);
    valid &= e == ESP_OK;
    if (!valid) {
        printf("[ERROR] SPI bus add device failed : %d\n", e);
    }

    //_spi_device = new SpiDevice(ss, _mode, _frequency, _queue_size);
    //return _spi_device->add_to_host(_host);
    return valid;
}

bool Master::end() {
    delete _spi_device;

    esp_err_t e = spi_bus_free(_host);
    if (e == ESP_OK) {
        return true;
    } else {
        printf("[ERROR] SPI bus free failed : %d\n", e);
        return false;
    }
}

uint8_t* Master::allocDMABuffer(const size_t s) {
    return (uint8_t*)heap_caps_malloc(s, MALLOC_CAP_DMA);
}

size_t Master::transfer(const uint8_t* tx_buf, const size_t size) {
    return transfer(tx_buf, NULL, size);
}

size_t Master::transfer(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size) {
    if (!_transactions.empty()) {
        std::stringstream ss;
        ss << "Cannot execute transfer if queued transaction exits. Queueed size = " << _transactions.size();
        throw std::runtime_error(ss.str());
    }

    addTransaction(tx_buf, rx_buf, size);

    // send a spi transaction, wait for it to complete, and return the result
    esp_err_t e = spi_device_transmit(_handle, &_transactions.back());
    if (e == ESP_OK) {
        size_t len = _transactions.back().rxlength / 8;
        _transactions.pop_back();
        return len;
    } else {
        printf("[ERROR] SPI device transmit failed : %d\n", e);
        _transactions.pop_back();
        return 0;
    }
}

bool Master::queue(const uint8_t* tx_buf, const size_t size) {
    return queue(tx_buf, NULL, size);
}

bool Master::queue(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size) {
    if (_transactions.size() >= _queue_size) {
        printf("[WARNING] queue is full with transactions. discard new transaction request\n");
        return false;
    } else {
        addTransaction(tx_buf, rx_buf, size);
        esp_err_t e = spi_device_queue_trans(_handle, &_transactions.back(), portMAX_DELAY);

        return (e == ESP_OK);
    }
}

void Master::yield() {
    size_t n = _transactions.size();
    for (uint8_t i = 0; i < n; ++i) {
        spi_transaction_t* r_trans;
        esp_err_t e = spi_device_get_trans_result(_handle, &r_trans, portMAX_DELAY);
        if (e != ESP_OK) {
            printf("[ERROR] SPI device get trans result failed %d / %d : %d\n", i, n, e);
        }
        _transactions.pop_front();
    }
}

void Master::setDataMode(const uint8_t mode) {
    _mode = mode;
};

void Master::setFrequency(const uint32_t frequency) {
    _frequency = frequency;
}

void Master::setMaxTransferSize(const int max_size) {
    _max_size = max_size;
}

void Master::setDMAChannel(const int channel) {
    _dma_chan = channel;  // 1 or 2 only
}

void Master::setQueueSize(const int queue_size) {
    _queue_size = queue_size;
}

void Master::addTransaction(const uint8_t* tx_buf, uint8_t* rx_buf, const size_t size) {
    _transactions.emplace_back(spi_transaction_t());
    _transactions.back().flags = 0;
    // _transactions.back().cmd = ;
    // _transactions.back().addr = ;
    _transactions.back().length = 8 * size;  // in bit size
    // _transactions.back().user = ;
    _transactions.back().tx_buffer = tx_buf;
    _transactions.back().rx_buffer = rx_buf;
}

ARDUINO_ESP32_DMA_SPI_NAMESPACE_END
