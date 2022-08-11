#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

#define MOTOR_SPIN_CHECK_TIME_DELAY 700
#define MOTOR_SPIN_CHECK_THRESHOLD 5

class MotorService {
    public:
        MotorService();

        MotorService(const int motorPin, const int motorSpeedCommandId);

        ~MotorService();

        void printInit() ;

        void startMotor();

        void stopMotor();

        void spinMotor();

        bool set_rotation_speed(const int16_t id, const int16_t rotation_speed);

    private:
        const int kMotorPin;
        const int k_motorSpeedCommandId;
        volatile int16_t _rotation_speed = 0;
};

#endif // MOTOR_H
