#include <serial_logger.h>
#include "esp32_ps4_controller.h"
#include "esp32_spi_master.h"
#include "engine_controller.h"
#include "object_detection_controller.h"

// General SPI settings
const int MOSI_PIN_GREEN  = 23;
const int MISO_PIN_YELLOW =  19;
const int SCK_PIN_ORANGE =  18;
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;

// Engine SPI slave settings
const int ENGINE_CONTROL_SS_PIN_BLUE = 5;
const int ENGINE_POWER_SUPPLY_PIN = 13;
const int ENGINE_POWER_BOOT_DELAY = 1000;
const int ENGINE_INTER_TRANSACTION_DELAY_MICROSECONDS = 10;
const int engine_control_unit_interval = 300;

// Object Detection SPI slave settings
const int OBJECT_DETECTION_CONTROL_SS_PIN_BROWN = 12;
const int OBJECT_DETECTION_POWER_SUPPLY_PIN = 14;
const int OBJECT_DETECTION_POWER_BOOT_DELAY = 1000;
const int OBJECT_DETECTION_INTER_TRANSACTION_DELAY_MICROSECONDS = 10;
const int object_detection_control_unit_interval = 150;

// General processing + PS4 (Bluetooth) settings
auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl = nullptr;
Esp32SpiMaster *esp32_spi_master = nullptr;
EngineController *engine_controller;
ObjectDetectionController *object_detection_controller;

void re_setup_spi_communication() {
    // delete the master will tear down all slaves (inklusive their power supply)
    SerialLogger::info("Shutting down all previous slaves");
    delete esp32_spi_master;
    SerialLogger::info("(Re)Setting up all slaves");
    esp32_spi_master = new Esp32SpiMaster(SCK_PIN_ORANGE, MISO_PIN_YELLOW, MOSI_PIN_GREEN, &re_setup_spi_communication,
                                          frequency);
    esp32_spi_master->addSlave(ENGINE_CONTROL_SS_PIN_BLUE, ENGINE_POWER_SUPPLY_PIN,
                               ENGINE_POWER_BOOT_DELAY, ENGINE_INTER_TRANSACTION_DELAY_MICROSECONDS,
                               engine_control_unit_interval, clock_divide, _timer, engine_controller);
    esp32_spi_master->addSlave(OBJECT_DETECTION_CONTROL_SS_PIN_BROWN, OBJECT_DETECTION_POWER_SUPPLY_PIN,
                               OBJECT_DETECTION_POWER_BOOT_DELAY,
                               OBJECT_DETECTION_INTER_TRANSACTION_DELAY_MICROSECONDS,
                               object_detection_control_unit_interval, clock_divide, _timer,
                               object_detection_controller);
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
    esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);
    engine_controller = new EngineController(esp32Ps4Ctrl);
    object_detection_controller = new ObjectDetectionController();

    re_setup_spi_communication();
}


void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
