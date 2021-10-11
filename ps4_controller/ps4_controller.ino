#include <SerialLogger.h>
#include "ESP32_PS4_Controller.h"
#include "esp32_spi_master.h"

void ps4_bt_controller_callback(const int slavePin, const int delayPerSpiRead, SPIClass *spiClass) {
    SerialLogger::info("SPI master send-receive to-from slave connected to pin %d", slavePin);

    char buf [20];
    const char readTransfer = spiClass->transfer(1);   // initiate transmission
    for (int pos = 0; pos < sizeof (buf) - 1; pos++) {
        // TODO can this be lower? Is transfer not blocking already?
        delayMicroseconds (delayPerSpiRead);
        buf [pos] = spiClass->transfer(0);
        if (buf [pos] == 0) {
            break;
        }
    }

    buf [sizeof (buf) - 1] = 0;  // ensure terminating null
    SerialLogger::info("SPI master received: %s", buf);
}

#define MOSI  23
#define MISO  19
#define SCK 18
#define SS  5
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";

ESP32_PS4_Controller *esp32Ps4Ctrl;

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    //esp32Ps4Ctrl = new ESP32_PS4_Controller (_timer, masterMac);
    Esp32SpiMaster spiMaster (SCK, MISO, MOSI, frequency);
    spiMaster.addSlave(SS, 100, clock_divide, _timer, &ps4_bt_controller_callback);
}

void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
