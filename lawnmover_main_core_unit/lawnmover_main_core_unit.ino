#include <serial_logger.h>
#include "esp32_ps4_controller.h"
#include "esp32_spi_master.h"
#include "engine_controller.h"

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
EngineController *engine_controller;

void re_setup_spi_communication() {
    // delete the master will tear down all slaves (inklusive their power supply)
    SerialLogger::info("Shutting down all previous slaves");
    delete esp32_spi_master;
    SerialLogger::info("(Re)Setting up all slaves");
    esp32_spi_master = new Esp32SpiMaster(SCK_PIN, MISO_PIN, MOSI_PIN, &re_setup_spi_communication, frequency);
    esp32_spi_master->addSlave(ENGINE_CONTROL_SS_PIN, ENGINE_POWER_SUPPLY_PIN, ENGINE_POWER_BOOT_DELAY,
                               ENGINE_INTER_TRANSACTION_DELAY_MICROSECONDS, engine_control_unit_interval,
                               clock_divide, _timer, engine_controller);
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
    esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);
    engine_controller = new EngineController(esp32Ps4Ctrl);

    re_setup_spi_communication();
}


void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
