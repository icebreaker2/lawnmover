#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <arduino_timer_uno.h>

#define MOTOR_SPIN_CHECK_TIME_DELAY 700
#define MOTOR_SPIN_CHECK_THRESHOLD 5

class MotorService {
    public:
        MotorService();

        MotorService(const int motorPin, Timer<> &timer, const bool deadMansSwitch);

        ~MotorService();

        void printInit() ;

        void startMotor();

        void stopMotor();

        void spinMotor();

        void checkAndResetMotorCmd();

    private:
        const int kMotorPin;
        volatile int _motorSpinCmdReceived = 0;
};

#endif // MOTOR_H
