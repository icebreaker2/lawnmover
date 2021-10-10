#ifndef ESP32_PS4_CONTROLLER_H
#define ESP32_PS4_CONTROLLER_H

#include <Arduino.h>
#include <arduino_timer_esp32.h>

class ESP32_PS4_Controller {
    public:
        ESP32_PS4_Controller(Timer<> &timer, const char *masterMac, const unsigned long timerDelay = 100,
                             const int readyPin = 2, const int connectedPin = 2,
                             const int commandReceivedPin = 2, const bool debug = false);

        ~ESP32_PS4_Controller();

    private:
        bool readState();
        bool checkCommandStates();
        bool checkAuxiliaryStates();

        const char *k_masterMac;
        const unsigned long k_timerDelay;

        const int k_readyPin;
        const int k_connectedPin;
        const int k_commandReceivedPin;

        const bool k_debug;
        bool _connected = false;

        bool m_isCharging = false;
        bool m_isAudioConnected = false;
        bool m_isMicConnected = false;
        int m_batteryLevel = -1;

        int m_lStickX = 0;
        int m_lStickY = 0;
        int m_rStickX = 0;
        int m_rStickY = 0;
};

#endif // ESP32_PS4_CONTROLLER_H
