#ifndef MOVER_H
#define MOVER_H

#include <Arduino.h>

// TODO make MovementService
#define MOVEMENT_DURATION 500

class MoverService {
    public:
        MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin, const int rightFwdPin,
                     const int rightBwdPin, const int leftFwdPwm, const int leftBwdPwm, const int rightFwdPwm, const int rightBwdPwm);
        ~MoverService();

        void printInit();

        void stopMovement();

        void turnLeft();

        void turnRight();

        void moveForward();

        void moveBackward();

        void changeLeftPwmRate(const int rate);

        void changeRightPwmRate(const int rate);

    private:
        const int kLeftFwdPin;
        const int kLeftBwdPin;
        const int kLeftPwmPin;
        const int kRightPwmPin;
        const int kRightFwdPin;
        const int kRightBwdPin;

        const int kLeftFwdPwm;
        const int kLeftBwdPwm;
        const int kRightFwdPwm;
        const int kRightBwdPwm;

        int currentLeftPwm = 0;
        int currentRightPwm = 0;
};

#endif // MOVER_H
