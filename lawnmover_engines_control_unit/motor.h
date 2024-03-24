#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <arduino_timer_uno.h>

#define MOTOR_SPIN_CHECK_TIME_DELAY 700
#define MOTOR_SPIN_CHECK_THRESHOLD 5

class MotorService {
public:
	MotorService();

	MotorService(const int motorPin);

	~MotorService();

	void printInit();

	void startMotor();

	void stopMotor();

	void interpretState();

	bool set_rotation_speed(const int16_t id, const int16_t rotation_speed);

    void scheduleInterpretingState(Timer<> &timer, const int interval) {
        timer.every(interval, [](MotorService *self) -> bool {
            self->interpretState();
            return true; // to repeat the action - false to stop
        }, this);
    };


private:
	const int kMotorPin;
	volatile int16_t _rotation_speed = 0;
    volatile int16_t _last_rotation_speed = 0;
};

#endif // MOTOR_H
