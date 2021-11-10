#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include <Arduino.h>
#include <SPI.h>
#include <driver/spi_master.h>
#include <deque>

class SpiDevice {
    public:
        SpiDevice(const int8_t ss, const uint8_t mode, const uint32_t frequency, const int queue_size) {
            _if_cfg.mode = mode;
            _if_cfg.clock_speed_hz = frequency;
            _if_cfg.queue_size = queue_size;
            _if_cfg.flags = SPI_DEVICE_NO_DUMMY;
            // TODO: callback??
            _if_cfg.pre_cb = NULL;
            _if_cfg.post_cb = NULL;

            _if_cfg.spics_io_num = ss;
        };

        ~SpiDevice() {
            esp_err_t e = spi_bus_remove_device(_handle);
            if (e != ESP_OK) {
                printf("[ERROR] SPI bus remove device failed : %d\n", e);
            }
        };

        bool add_to_host(spi_host_device_t &host) {
            esp_err_t e = spi_bus_add_device(host, &_if_cfg, &_handle);
            bool valid = e == ESP_OK;
            if (!valid) {
                printf("[ERROR] SPI bus add device failed : %d\n", e);
            }
            return valid;
        };

        spi_device_interface_config_t &get_spi_device_interface_config() {
            return _if_cfg;
        };

        spi_device_handle_t &get_spi_device_handle() {
            return _handle;
        };

    private:
        spi_device_interface_config_t _if_cfg;
        spi_device_handle_t _handle;

};

#endif // SPI_DEVICE_H
