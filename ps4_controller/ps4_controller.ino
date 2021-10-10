#include <SerialLogger.h>
#include "ESP32_PS4_Controller.h"

auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";

ESP32_PS4_Controller *esp32Ps4Ctrl;

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    esp32Ps4Ctrl = new ESP32_PS4_Controller (_timer, masterMac);
}

void loop() {
    // tick timers
    auto ticks = _timer.tick();
}
