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

        bool m_charging = false;
        bool m_audioConnected = false;
        bool m_micConnected = false;
        int m_batteryLevel = -1;

        volatile int m_lStickX = 0;
        volatile int m_lStickY = 0;
        volatile int m_rStickX = 0;
        volatile int m_rStickY = 0;

        volatile bool m_downButtonPressed = false;
        volatile bool m_upButtonPressed = false;
        volatile bool m_leftButtonPressed = false;
        volatile bool m_rightButtonPressed = false;

        volatile bool m_squareButtonPressed = false;
        volatile bool m_crossButtonPressed = false;
        volatile bool m_circleButtonPressed = false;
        volatile bool m_triangleButtonPressed = false;
        volatile bool m_lbButtonPressed = false;
        volatile bool m_rbButtonPressed = false;
        volatile bool m_ltButtonPressed = false;
        volatile int m_ltValue = 0; 
        volatile bool m_rtButtonPressed = false;
        volatile int m_rtValue = 0;
        volatile bool m_l3ButtonPressed = false;
        volatile bool m_r3ButtonPressed = false;
        volatile bool m_shareButtonPressed = false;
        volatile bool m_optionsButtonPressed = false;
        volatile bool m_psButtonPressed = false;
        volatile bool m_touchpadButtonPressed = false;

};

#endif // ESP32_PS4_CONTROLLER_H
