#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>

// TODO make MovementService
#define MOVEMENT_DURATION 500

class MoverService {
    public:
        MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin, const int rightFwdPin,
                     const int rightBwdPin);
        ~MoverService();

        void printInit();

        void printState();

        bool set_left_wheels_power(const int16_t wheels_power) {
            left_wheels_power = wheels_power;
            return true;
        };

        bool set_right_wheels_power(const int16_t wheels_power) {
            right_wheels_power = wheels_power;
            return true;
        };

        void interpret_state();
    private:
        const int kLeftFwdPin;
        const int kLeftBwdPin;
        const int kLeftPwmPin;
        const int kRightPwmPin;
        const int kRightFwdPin;
        const int kRightBwdPin;

        volatile int left_wheels_power;
        volatile int right_wheels_power;

        void stopMovement();

        void changeLeftPwmRate(const int rate);

        void changeRightPwmRate(const int rate);
};

#endif // MOVER_H
