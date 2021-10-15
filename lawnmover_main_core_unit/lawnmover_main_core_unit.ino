#include <serial_logger.h>
#include "esp32_ps4_controller.h"
#include "esp32_spi_master.h"

// TODO make system lib
#include "spi_commands.h"

const int MOSI_PIN  = 23;
const int MISO_PIN =  19;
const int SCK_PIN =  18;
const int ENGINE_CONTROL_SS_PIN = 5;
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;
const int engine_control_unit_interval = 200;

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl;


long engine_control_tx_buffer_size = 3 * (COMMAND_FRAME_SIZE);
uint8_t *engine_control_tx_buffer = new uint8_t[engine_control_tx_buffer_size];
uint8_t *engine_control_unit_supplier(long &buffer_size) {
    SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, esp32Ps4Ctrl->getLStickY(), engine_control_tx_buffer);
    engine_control_tx_buffer[COMMAND_FRAME_SIZE - 1] = 0xFF;

    SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, esp32Ps4Ctrl->getRStickY(), engine_control_tx_buffer + COMMAND_FRAME_SIZE);
    engine_control_tx_buffer[COMMAND_FRAME_SIZE * 2 - 1] = 0xFF;

    SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, esp32Ps4Ctrl->getRtValue(), engine_control_tx_buffer + 2 * COMMAND_FRAME_SIZE);
    engine_control_tx_buffer[COMMAND_FRAME_SIZE * 3 - 1] = 0xFF;

    buffer_size = engine_control_tx_buffer_size;
    return engine_control_tx_buffer;
}

bool engine_control_unit_consumer(uint8_t *slave_response_buffer, long buffer_size) {
    bool valid = true;
    if (buffer_size != engine_control_tx_buffer_size) {
        SerialLogger::error("Cannot consume slave output. Buffer size does not match written bytes to slave");
        valid = false;
    } else {
        valid = SpiCommands::master_interpret_communication(engine_control_tx_buffer, slave_response_buffer, buffer_size);
    }

    if (!valid) {
        SerialLogger::error("Shutting slave on slave-select pin %d down!", ENGINE_CONTROL_SS_PIN);
        // TODO set power supply for slave to low
    }
    return valid;
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);
    Esp32SpiMaster esp32_spi_master(SCK_PIN, MISO_PIN, MOSI_PIN, frequency);
    esp32_spi_master.addSlave(ENGINE_CONTROL_SS_PIN, engine_control_unit_interval, clock_divide, _timer,
                              &engine_control_unit_supplier, &engine_control_unit_consumer);
}


void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
