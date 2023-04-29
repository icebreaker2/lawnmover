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

#include <sstream>
#include "spi_slave_handler.h"
#include "serial_logger.h"

spi_bus_config_t SpiSlaveHandler::_bus_cfg;
bool SpiSlaveHandler::_initialized_once = false;
spi_host_device_t SpiSlaveHandler::_host = HSPI_HOST;
uint8_t SpiSlaveHandler::_mode = SPI_MODE3;
uint32_t SpiSlaveHandler::_frequency = SPI_MASTER_FREQ_8M;
int SpiSlaveHandler::_queue_size = 1;
int SpiSlaveHandler::_dma_chan = 0;     // must be 1 or 2 or 0 if deactivated (limits transaction size to 64 bytes)
int SpiSlaveHandler::_max_size = 4094;  // default size
std::deque <spi_transaction_t> SpiSlaveHandler::_transactions; // Default init with constructor

bool SpiSlaveHandler::init_bus(const int8_t sck, const int8_t miso, const int8_t mosi, const uint8_t spi_bus) {
	if (_initialized_once) {
		SerialLogger::info(F("Not initializing SPI bus again. Already initialized."));
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

bool
SpiSlaveHandler::begin(const int8_t sck, const int8_t miso, const int8_t mosi, const int8_t ss, const uint8_t spi_bus) {
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

	return valid;
}

bool SpiSlaveHandler::end() {
	esp_err_t e = spi_bus_free(_host);
	if (e == ESP_OK) {
		return true;
	} else {
		printf("[ERROR] SPI bus free failed : %d\n", e);
		return false;
	}
}

uint8_t *SpiSlaveHandler::allocDMABuffer(const size_t s) {
	return (uint8_t *) heap_caps_malloc(s, MALLOC_CAP_DMA);
}

void SpiSlaveHandler::transfer(const uint8_t *tx_buf, const size_t size) {
	transfer(tx_buf, NULL, size);
}

void SpiSlaveHandler::transfer(const uint8_t *tx_buf, uint8_t *rx_buf, const size_t size) {
	if (_initialized_once) {
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
		} else {
			printf("[ERROR] SPI device transmit failed : %d\n", e);
			_transactions.pop_back();
		}
		delayMicroseconds(k_inter_transaction_delay_microseconds);
	}
}

void SpiSlaveHandler::setDataMode(const uint8_t mode) {
	if (_initialized_once) {
		SerialLogger::info(F("Not setting data mode again. Already initialized."));
	} else {
		_mode = mode;
	}
};

void SpiSlaveHandler::setFrequency(const uint32_t frequency) {
	if (_initialized_once) {
		SerialLogger::info(F("Not setting frequency again. Already initialized."));
	} else {
		_frequency = frequency;
	}
}

void SpiSlaveHandler::setMaxTransferSize(const int max_size) {
	if (_initialized_once) {
		SerialLogger::info(F("Not setting max_size again. Already initialized."));
	} else {
		_max_size = max_size;
	}
}

void SpiSlaveHandler::setDMAChannel(const int channel) {
	if (_initialized_once) {
		SerialLogger::info(F("Not setting DMA channel again. Already initialized."));
	} else {
		_dma_chan = channel;  // 1 or 2 only or 0 to disable
	}
}

void SpiSlaveHandler::addTransaction(const uint8_t *tx_buf, uint8_t *rx_buf, const size_t size) {
	_transactions.emplace_back(spi_transaction_t());
	_transactions.back().flags = 0;
	// _transactions.back().cmd = ;
	// _transactions.back().addr = ;
	_transactions.back().length = 8 * size;  // in bit size
	// _transactions.back().user = ;
	_transactions.back().tx_buffer = tx_buf;
	_transactions.back().rx_buffer = rx_buf;
}
