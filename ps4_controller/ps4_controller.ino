#include "ESP32_PS4_Controller.h"

//auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";

ESP32_PS4_Controller *esp32Ps4Ctrl;

void setup() { 
  esp32Ps4Ctrl = new ESP32_PS4_Controller (masterMac);
}

void loop() {
  // read with delay
  esp32Ps4Ctrl->waitAndRead();
}
