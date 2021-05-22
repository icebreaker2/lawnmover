#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <arduino-timer.h>

#define LED_INTRA_ROTATION_TIME_DELAY 100

class Led3Service {
    public:
        Led3Service(const int led1Pin, const int led2Pin, const int led3Pin, Timer<> &timer);

        ~Led3Service();

    private:
        const int kLed1Pin;
        const int kLed2Pin;
        const int kLed3Pin;

        volatile int _nextState = 0;
        Timer<> _timer;
};

#endif // LED_H
