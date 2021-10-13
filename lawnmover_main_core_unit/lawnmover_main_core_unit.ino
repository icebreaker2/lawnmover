#include <serial_logger.h>
#include "esp32_ps4_controller.h"

#define MOSI  23
#define MISO  19
#define SCK 18
#define SS  5
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";

ESP32_PS4_Controller *esp32Ps4Ctrl;

#include <ESP32DMASPIMaster.h>
#include <ESP32DMASPISlave.h>

ESP32DMASPI::Master master;
//ESP32DMASPI::Slave slave;

// Reference: https://rabbit-note.com/2019/01/20/esp32-arduino-spi-slave/
static const uint32_t BUFFER_SIZE = 20;
uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;
//uint8_t* spi_slave_tx_buf;
//uint8_t* spi_slave_rx_buf;

void set_buffer() {
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        spi_master_tx_buf[i] = (i % 12) & 0xFF;
        //spi_master_tx_buf[i] = (i % 2) & 0xFF;
        //spi_slave_tx_buf[i] = (0xFF - i) & 0xFF;
    }
    memset(spi_master_rx_buf, 0, BUFFER_SIZE);
    //memset(spi_slave_rx_buf, 0, BUFFER_SIZE);
}


void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    esp32Ps4Ctrl = new ESP32_PS4_Controller (_timer, masterMac);

    // to use DMA buffer, use these methods to allocate buffer
    spi_master_tx_buf = master.allocDMABuffer(BUFFER_SIZE);
    spi_master_rx_buf = master.allocDMABuffer(BUFFER_SIZE);
    //spi_slave_tx_buf = slave.allocDMABuffer(BUFFER_SIZE);
    //spi_slave_rx_buf = slave.allocDMABuffer(BUFFER_SIZE);

    set_buffer();

    delay(1000);

    master.setDataMode(SPI_MODE0);
    // master.setFrequency(SPI_MASTER_FREQ_8M); // too fast for bread board...
    master.setFrequency(10000);
    master.setMaxTransferSize(BUFFER_SIZE);
    // Disabling DMA limits to 64 bytes per transaction only
    master.setDMAChannel(0);  // 1 or 2 only
    master.setQueueSize(10);   // transaction queue size
    // begin() after setting
    // VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
    master.begin(VSPI);

    //slave.setDataMode(SPI_MODE3);
    //slave.setMaxTransferSize(BUFFER_SIZE);
    //slave.setDMAChannel(2);  // 1 or 2 only
    //slave.setQueueSize(1);   // transaction queue size
    // begin() after setting
    // HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12
    //slave.begin(HSPI);

    // connect same name pins each other
    // CS - CS, CLK - CLK, MOSI - MOSI, MISO - MISO
}

long counter = BUFFER_SIZE;
const int chunk_size = 1;
void loop() {
    // set buffer data here
    // TODO set buffer
    // start and wait to complete transaction
    uint8_t *rxPointer;
    uint8_t *txPointer;
    if (counter < BUFFER_SIZE) {
        rxPointer = spi_master_rx_buf + counter;
        txPointer = spi_master_tx_buf + counter;
    } else {
        rxPointer = spi_master_rx_buf;
        txPointer  = spi_master_tx_buf;
        counter = 0;
    }

    Serial.printf("Transfering %d (counter: %d)\n", txPointer[0], counter);
    size_t sendBytes = master.transfer(txPointer, rxPointer, chunk_size);
    Serial.printf("Received %d bytes\n", sendBytes);
    // do something here with received data (if needed)
    Serial.printf("Rx internal status: ");
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        Serial.printf("%c", spi_master_rx_buf[i]);
    }
    Serial.printf("\n");
    /*  Serial.printf("Tx internal status: ");
        for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        Serial.printf("%d ", spi_master_tx_buf[i]);
        }
        Serial.printf("\n");
    */
    delay(1);
    counter += chunk_size;
    // tick timers
    auto ticks = _timer.tick();
}
