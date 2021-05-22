#include "LED.h"


Led3Service::Led3Service(const int led1Pin, const int led2Pin, const int led3Pin, Timer<> &timer)  :
    kLed1Pin(led1Pin), kLed2Pin(led2Pin), kLed3Pin(led3Pin) {
    _timer = timer;

    // LEDs (all on initially)
    pinMode(kLed1Pin, OUTPUT);
    pinMode(kLed2Pin, OUTPUT);
    pinMode(kLed3Pin, OUTPUT);
    digitalWrite(kLed1Pin, HIGH);
    digitalWrite(kLed2Pin, HIGH);
    digitalWrite(kLed3Pin, HIGH);

    // schedule blinking rotation
    _timer.every(LED_INTRA_ROTATION_TIME_DELAY, [](void*) -> bool {
        //        switch (_nextState) {
        //            case 0:
        //                digitalWrite(kLed1Pin, HIGH);
        //                digitalWrite(kLed2Pin, HIGH);
        //                digitalWrite(kLed3Pin, LOW);
        //                _nextState++;
        //                Serial.println("1/6");
        //                break;
        //            case 1:
        //                digitalWrite(kLed1Pin, HIGH);
        //                digitalWrite(kLed2Pin, LOW);
        //                digitalWrite(kLed3Pin, LOW);
        //                _nextState++;
        //                Serial.println("2/6");
        //                break;
        //            case 2:
        //                digitalWrite(kLed1Pin, LOW);
        //                digitalWrite(kLed2Pin, LOW);
        //                digitalWrite(kLed3Pin, LOW);
        //                _nextState++;
        //                Serial.println("3/6");
        //                break;
        //            case 3:
        //                digitalWrite(kLed1Pin, LOW);
        //                digitalWrite(kLed2Pin, LOW);
        //                digitalWrite(kLed3Pin, HIGH);
        //                _nextState++;
        //                Serial.println("4/6");
        //                break;
        //            case 4:
        //                digitalWrite(kLed1Pin, LOW);
        //                digitalWrite(kLed2Pin, HIGH);
        //                digitalWrite(kLed3Pin, HIGH);
        //                _nextState++;
        //                Serial.println("5/6");
        //                break;
        //            case 5:
        //                digitalWrite(kLed1Pin, HIGH);
        //                digitalWrite(kLed2Pin, HIGH);
        //                digitalWrite(kLed3Pin, HIGH);
        //                _nextState = 0;
        //                Serial.println("6/6");
        //                break;
        //        }
        return true; // to repeat the action - false to stop
    });
}

Led3Service::~Led3Service() {
    digitalWrite(kLed1Pin, LOW);
    digitalWrite(kLed2Pin, LOW);
    digitalWrite(kLed3Pin, LOW);
}
