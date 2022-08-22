#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>
#include <serial_logger.h>
#include <spi_commands.h>

// TODO make MovementService
#define MOVEMENT_DURATION 500

class MoverService {
    public:
        MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin,
                     const int rightFwdPin, const int rightBwdPin);
        ~MoverService();

        void printInit();

        void printState();

        bool set_left_wheels_power(const int16_t id, const int16_t wheels_power) {
            // Logging (serial printing is faster) must be kept to an absolute minimum for this SPI command callback depending on the logging baudrate 
            // SerialLogger::debug("Inspecting left wheels power with id %d and value %d", id, wheels_power);
            if (id == LEFT_WHEEL_STEERING_COMMAND) {
                left_wheels_power = wheels_power;
                return true;
            } else {
                return false;
            }
        };

        bool set_right_wheels_power(const int16_t id, const int16_t wheels_power) {
            // Logging (serial printing is faster) must be kept to an absolute minimum for this SPI command callback depending on the logging baudrate 
            // SerialLogger::debug("Inspecting right wheels power with id %d and value %d", id, wheels_power);
            if (id == RIGHT_WHEEL_STEERING_COMMAND) {
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

        volatile int left_wheels_power = 0;
        volatile int right_wheels_power = 0;

        void stopMovement();

        void changeLeftPwmRate(const int rate);

        void changeRightPwmRate(const int rate);
};

#endif // MOVER_H
