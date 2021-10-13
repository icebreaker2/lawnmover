#include <serial_logger.h>
#include "esp32_ps4_controller.h"
#include "esp32_spi_master.h"

// TODO make system lib
#include "spi_commands.h"

const int MOSI_PIN  = 23;
const int MISO_PIN =  19;
const int SCK_PIN =  18;
const int SS_PIN = 5;
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;
const int engine_control_unit_interval = 10;

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl;

uint8_t *engine_control_tx_buffer;
long engine_control_tx_buffer_size;
uint8_t *engine_control_unit_supplier(long &buffer_size) {
    buffer_size = engine_control_tx_buffer_size;
    return engine_control_tx_buffer;
}

bool engine_control_unit_consumer(uint8_t *slave_response_buffer, long buffer_size) {
    // TODO check if the responses match the specific type
    // TODO return false upon failure in verification (causing the master to disconnect the slave (from power))
    // TODO return true only on success
    return true;
}

void setup_buffer() {
    engine_control_tx_buffer = new uint8_t[SpiCommands::ENGINE_CONTROL_UNIT_BUFFER_SIZE];
    engine_control_tx_buffer_size = sizeof(engine_control_tx_buffer) / sizeof(engine_control_tx_buffer[0]);
}

uint8_t *example_tx_buffer = new uint8_t[12] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
long example_tx_buffer_size = 12;
uint8_t *example_supplier(long &buffer_size) {
    buffer_size = example_tx_buffer_size;
    return example_tx_buffer;
}

bool example_consumer(uint8_t *slave_response_buffer, long buffer_size) {
    Serial.printf("Rx internal status: ");
    for (size_t i = 0; i < buffer_size; ++i) {
        Serial.printf("%c", slave_response_buffer[i]);
    }
    Serial.printf("\n");
    Serial.printf("Tx internal status: ");
    for (size_t i = 0; i < example_tx_buffer_size; ++i) {
        Serial.printf("%d ", example_tx_buffer[i]);
    }
    Serial.printf("\n");
    return true;
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    setup_buffer();
    esp32Ps4Ctrl = new ESP32_PS4_Controller(_timer, masterMac);
    Esp32SpiMaster esp32_spi_master(SCK_PIN, MISO_PIN, MOSI_PIN, frequency);
    //esp32_spi_master.addSlave(SS_PIN, engine_control_unit_interval, clock_divide, _timer,
    //                         &engine_control_unit_supplier, &engine_control_unit_consumer);
    esp32_spi_master.addSlave(SS_PIN, engine_control_unit_interval, clock_divide, _timer,
                              &example_supplier, &example_consumer);
}


void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
