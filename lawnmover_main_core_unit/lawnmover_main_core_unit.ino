#include <serial_logger.h>
#include "esp32_ps4_controller.h"
#include "esp32_spi_master.h"

#include <spi_commands.h>

const int MOSI_PIN  = 23;
const int MISO_PIN =  19;
const int SCK_PIN =  18;
const int ENGINE_CONTROL_SS_PIN = 5;
const int ENGINE_POWER_SUPPLY_PIN = 13;
const int ENGINE_POWER_BOOT_DELAY = 1000;
const int ENGINE_INTER_TRANSACTION_DELAY_MICROSECONDS = 10;

const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;
const int engine_control_unit_interval = 200;

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl = nullptr;
Esp32SpiMaster *esp32_spi_master = nullptr;

long engine_control_tx_buffer_size = 3 * (COMMAND_FRAME_SIZE);
uint8_t *engine_control_tx_buffer = new uint8_t[engine_control_tx_buffer_size];
bool engine_slave_synchronized = false;

uint8_t *engine_control_unit_supplier(long &buffer_size) {
    if (engine_slave_synchronized) {
        SpiCommands::putCommandToBuffer(LEFT_WHEEL_STEERING_COMMAND, esp32Ps4Ctrl->getLStickY(), engine_control_tx_buffer);
        SpiCommands::putCommandToBuffer(RIGHT_WHEEL_STEERING_COMMAND, esp32Ps4Ctrl->getRStickY(), engine_control_tx_buffer + COMMAND_FRAME_SIZE);
        SpiCommands::putCommandToBuffer(MOTOR_SPEED_COMMAND, esp32Ps4Ctrl->getRtValue(), engine_control_tx_buffer + 2 * COMMAND_FRAME_SIZE);

        buffer_size = engine_control_tx_buffer_size;
        return engine_control_tx_buffer;
    } else {
        buffer_size = COMMUNICATION_START_SEQUENCE_LENGTH;
        return SpiCommands::COMMUNICATION_START_SEQUENCE;
    }
}

bool engine_control_unit_consumer(uint8_t *slave_response_buffer, long buffer_size) {
    bool valid = true;
    Serial.print("RxBufferInput:");
    for (long i = 0; i < buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(slave_response_buffer[i], HEX);
    }
    Serial.println();


    Serial.print("TxBufferInput:");
    for (long i = 0; i < buffer_size; i += 1) {
        if (i % COMMAND_FRAME_SIZE == 0) {
            Serial.print(" ");
        }
        Serial.print(engine_control_tx_buffer[i], HEX);
    }
    Serial.println();

    if (engine_slave_synchronized) {
        if (buffer_size != engine_control_tx_buffer_size) {
            SerialLogger::error("Cannot consume slave output. Buffer size does not match written bytes to slave");
            valid = false;
        } else {
            valid = SpiCommands::master_interpret_communication(engine_control_tx_buffer, slave_response_buffer, buffer_size);
        }
    } else {
        if (buffer_size != COMMUNICATION_START_SEQUENCE_LENGTH) {
            SerialLogger::error("Cannot consume slave output. Buffer size does not match written bytes to slave");
            valid = false;
        } else {
            // the latest (n+1) byte send is 0xFF which is needed to read the nth byte
            for (long i = 0; i < COMMUNICATION_START_SEQUENCE_LENGTH - 1; i += 1) {
                const char &rx_byte = slave_response_buffer[i + COMMAND_SPI_RX_OFFSET];
                if (rx_byte == SpiCommands::COMMUNICATION_START_SEQUENCE[i]) {
                    SerialLogger::debug("Slave rx byte on index %d is %x and does match expected byte %x ", i, rx_byte,
                                        SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                } else {
                    SerialLogger::warn("Slave rx byte on index %d is %x and does not match expected byte %x ", i, rx_byte,
                                       SpiCommands::COMMUNICATION_START_SEQUENCE[i]);
                    valid = false;
                    break;
                }
            }
        }
    }

    if (valid) {
        if (!engine_slave_synchronized) {
            SerialLogger::info("Engine Slave synchronized with this master");
        }
        engine_slave_synchronized = true;
    } else {
        if (engine_slave_synchronized) {
            SerialLogger::info("Engine Slave no longer synchronized with this master");
        } else {
            SerialLogger::info("Engine Slave not synchronized with this master");
        }
        engine_slave_synchronized = false;
    }

    return valid;
}

void re_setup_spi_communication() {
    // delete the master will tear down all slaves (inklusive their power supply)
    SerialLogger::info("Shutting down all previous slaves");
    delete esp32_spi_master;
    SerialLogger::info("(Re)Setting up all slaves");
    esp32_spi_master = new Esp32SpiMaster(SCK_PIN, MISO_PIN, MOSI_PIN, &re_setup_spi_communication, frequency);
    esp32_spi_master->addSlave(ENGINE_CONTROL_SS_PIN, ENGINE_POWER_SUPPLY_PIN, ENGINE_POWER_BOOT_DELAY,
                               ENGINE_INTER_TRANSACTION_DELAY_MICROSECONDS, engine_control_unit_interval,
                               clock_divide, _timer, &engine_control_unit_supplier, &engine_control_unit_consumer,
                               engine_slave_synchronized);
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
    esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);

    re_setup_spi_communication();
}


void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
