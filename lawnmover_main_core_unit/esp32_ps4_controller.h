#ifndef ESP32_PS4_CONTROLLER_H
#define ESP32_PS4_CONTROLLER_H

#include <Arduino.h>
#include <arduino_timer_esp32.h>

class ESP32_PS4_Controller {
    public:
        ESP32_PS4_Controller(const char *masterMac, Timer<> &timer, const unsigned long timerDelay = 100,
                             const int readyPin = 2, const int connectedPin = 2, const int commandReceivedPin = 2);

        ~ESP32_PS4_Controller();

        int16_t getLStickX () {
            return m_lStickX;
        };
        int16_t getLStickY () {
            return m_lStickY;
        };
        int16_t getRStickX () {
            return m_rStickX;
        };
        int16_t getRStickY () {
            return m_rStickY;
        };

        bool getDownButtonPressed() {
            return m_downButtonPressed;
        };
        bool getUpButtonPressed() {
            return m_upButtonPressed;
        };
        bool getLeftButtonPressed() {
            return m_leftButtonPressed;
        };
        bool getRightButtonPressed() {
            return m_rightButtonPressed;
        };

        bool getSquareButtonPressed() {
            return m_squareButtonPressed;
        };
        bool getCrossButtonPressed() {
            return m_crossButtonPressed;
        };
        bool getCircleButtonPressed() {
            return m_circleButtonPressed;
        };
        bool getTriangleButtonPressed() {
            return m_triangleButtonPressed;
        };
        bool getLbButtonPressed() {
            return m_lbButtonPressed;
        };
        bool getRbButtonPressed() {
            return m_rbButtonPressed;
        };
        bool getLtButtonPressed() {
            return m_ltButtonPressed;
        };
        int16_t getLtValue() {
            return m_ltValue;
        };
        bool getRtButtonPressed() {
            return m_rtButtonPressed;
        };
        int16_t getRtValue() {
            return m_rtValue;
        };
        bool getL3ButtonPressed() {
            return m_l3ButtonPressed;
        };
        bool getR3ButtonPressed() {
            return m_r3ButtonPressed;
        };
        bool getShareButtonPressed() {
            return m_shareButtonPressed;
        };
        bool getOptionsButtonPressed() {
            return m_optionsButtonPressed;
        };
        bool getPsButtonPressed() {
            return m_psButtonPressed;
        };
        bool getTouchpadButtonPressed() {
            return m_touchpadButtonPressed;
        };

        bool isConnected() {
            return _connected;
        };

    private:
        bool readState();
        bool checkCommandStates();
        bool checkAuxiliaryStates();

        const char *k_masterMac;
        const unsigned long k_timerDelay;

        const int k_readyPin;
        const int k_connectedPin;
        const int k_commandReceivedPin;

        const float k_stick_scaler = 128.0;
        bool _connected = false;

        bool m_charging = false;
        bool m_audioConnected = false;
        bool m_micConnected = false;
        int16_t m_batteryLevel = -1;

        volatile int16_t m_lStickX = 0;
        volatile int16_t m_lStickY = 0;
        volatile int16_t m_rStickX = 0;
        volatile int16_t m_rStickY = 0;

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
        volatile int16_t m_ltValue = 0;
        volatile bool m_rtButtonPressed = false;
        volatile int16_t m_rtValue = 0;
        volatile bool m_l3ButtonPressed = false;
        volatile bool m_r3ButtonPressed = false;
        volatile bool m_shareButtonPressed = false;
        volatile bool m_optionsButtonPressed = false;
        volatile bool m_psButtonPressed = false;
        volatile bool m_touchpadButtonPressed = false;
};

#endif // ESP32_PS4_CONTROLLER_H
