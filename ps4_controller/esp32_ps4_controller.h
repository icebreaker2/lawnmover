#ifndef ESP32_PS4_CONTROLLER_H
#define ESP32_PS4_CONTROLLER_H

#include <Arduino.h>
// Cannot use this library because it modifies the pins used by this board. Not compatible with Az Delivery ESP32 D1 R32 WROOM-32
// TODO find another timer library or check the actual timers on board (iff present)
#include <arduino_timer_esp32.h>

#include <functional>
#include <iostream>

class ESP32_PS4_Controller {
    public:
        ESP32_PS4_Controller(const char *masterMac, const unsigned long timerDelay = 100, const int readyPin = 2,
                             const int connectedPin = 2, const int commandReceivedPin = 2, const bool debug = false);

        ~ESP32_PS4_Controller();

        void waitAndRead();

    private:
        bool readState();

        const String k_masterMac;
        const unsigned long k_timerDelay;

        const int k_readyPin;
        const int k_connectedPin;
        const int k_commandReceivedPin;

        const bool k_debug;
        bool _connected = false;

        std::function<bool()> _internal_timer_handler;
};

#endif // ESP32_PS4_CONTROLLER_H
