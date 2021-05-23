#include "LED.h"

int _led1Pin = 0;
int _led2Pin = 0;
int _led3Pin = 0;
volatile int __nextState = 0;

Led3Service::Led3Service(const int led1Pin, const int led2Pin, const int led3Pin, Timer<> &timer)  :
    kLed1Pin(led1Pin), kLed2Pin(led2Pin), kLed3Pin(led3Pin) {

    // LEDs (all on initially)
    pinMode(kLed1Pin, OUTPUT);
    pinMode(kLed2Pin, OUTPUT);
    pinMode(kLed3Pin, OUTPUT);
    digitalWrite(kLed1Pin, HIGH);
    digitalWrite(kLed2Pin, HIGH);
    digitalWrite(kLed3Pin, HIGH);

    // TODO fix lambda objects to caputure / introducer but every did not allow that....Fix the timer lib
    _led1Pin = kLed1Pin;
    _led2Pin = kLed2Pin;
    _led3Pin = kLed3Pin;

    __nextState = _nextState;

    Serial.print("Scheduling blinking rotation at ");
    Serial.print(LED_INTRA_ROTATION_TIME_DELAY);
    Serial.print("ms for Pin: ");
    Serial.print(_led1Pin);
    Serial.print(", ");
    Serial.print(_led2Pin);
    Serial.print(", ");
    Serial.println(_led3Pin);

    timer.every(LED_INTRA_ROTATION_TIME_DELAY, [](void*) -> bool {
        switch (__nextState) {
            case 0:
                digitalWrite(_led1Pin, HIGH);
                digitalWrite(_led2Pin, HIGH);
                digitalWrite(_led3Pin, LOW);
                __nextState++;
//                Serial.println("Led turn: 1/6");
                break;
            case 1:
                digitalWrite(_led1Pin, HIGH);
                digitalWrite(_led2Pin, LOW);
                digitalWrite(_led3Pin, LOW);
                __nextState++;
//                Serial.println("Led turn: 2/6");
                break;
            case 2:
                digitalWrite(_led1Pin, LOW);
                digitalWrite(_led2Pin, LOW);
                digitalWrite(_led3Pin, LOW);
                __nextState++;
//                Serial.println("Led turn: 3/6");
                break;
            case 3:
                digitalWrite(_led1Pin, LOW);
                digitalWrite(_led2Pin, LOW);
                digitalWrite(_led3Pin, HIGH);
                __nextState++;
//                Serial.println("Led turn: 4/6");
                break;
            case 4:
                digitalWrite(_led1Pin, LOW);
                digitalWrite(_led2Pin, HIGH);
                digitalWrite(_led3Pin, HIGH);
                __nextState++;
//                Serial.println("Led turn: 5/6");
                break;
            case 5:
                digitalWrite(_led1Pin, HIGH);
                digitalWrite(_led2Pin, HIGH);
                digitalWrite(_led3Pin, HIGH);
                __nextState = 0;
//                Serial.println("Led turn: 6/6");
                break;
            default:
                Serial.print("Led turn: ");
                Serial.print(__nextState);
                Serial.println("/6");
        }
        return true; // to repeat the action - false to stop
    });
}

Led3Service::~Led3Service() {
    digitalWrite(kLed1Pin, LOW);
    digitalWrite(kLed2Pin, LOW);
    digitalWrite(kLed3Pin, LOW);
}
