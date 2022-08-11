#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>
#include <serial_logger.h>

// TODO make MovementService
#define MOVEMENT_DURATION 500

class MoverService {
    public:
        MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin,
                     const int rightFwdPin, const int rightBwdPin, const int leftWheelSteeringCommandId, 
                     const int rightWheelSteeringCommandId);
        ~MoverService();

        void printInit();

        void printState();

        bool set_left_wheels_power(const int16_t id, const int16_t wheels_power) {
            // Logging must be kept to an absolute minimum for this SPI command callback depending on the logging baudrate 
            // SerialLogger::debug("Inspecting left wheels power with id %d and value %d", id, wheels_power);
            if (id == k_leftWheelSteeringCommandId) {
                left_wheels_power = wheels_power;
                return true;
            } else {
                return false;
            }
        };

        bool set_right_wheels_power(const int16_t id, const int16_t wheels_power) {
            // Logging must be kept to an absolute minimum for this SPI command callback depending on the logging baudrate 
            // SerialLogger::debug("Inspecting right wheels power with id %d and value %d", id, wheels_power);
            if (id == k_rightWheelSteeringCommandId) {
                right_wheels_power = wheels_power;
                return true;
            } else {
                return false;
            }
        };

        void interpret_state();
    private:
        const int kLeftFwdPin;
        const int kLeftBwdPin;
        const int kLeftPwmPin;
        const int kRightPwmPin;
        const int kRightFwdPin;
        const int kRightBwdPin;

        const int k_leftWheelSteeringCommandId;
        const int k_rightWheelSteeringCommandId;

        volatile int left_wheels_power = 0;
        volatile int right_wheels_power = 0;

        void stopMovement();

        void changeLeftPwmRate(const int rate);

        void changeRightPwmRate(const int rate);
};

#endif // MOVER_H
