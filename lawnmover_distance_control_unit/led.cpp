#include "led.h"
#include <serial_logger.h>

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

  _led1Pin = kLed1Pin;
  _led2Pin = kLed2Pin;
  _led3Pin = kLed3Pin;

  __nextState = _nextState;
  SerialLogger::info("Scheduling blinking rotation at %d %s %d, %d, %d", LED_INTRA_ROTATION_TIME_DELAY, "ms for Pin:", _led1Pin, _led2Pin, _led3Pin);

  timer.every(LED_INTRA_ROTATION_TIME_DELAY, [](void*) -> bool {
    switch (__nextState) {
      case 0:
        digitalWrite(_led1Pin, HIGH);
        digitalWrite(_led2Pin, HIGH);
        digitalWrite(_led3Pin, LOW);
        __nextState++;
        SerialLogger::trace("Led turn: 1/6");
        break;
      case 1:
        digitalWrite(_led1Pin, HIGH);
        digitalWrite(_led2Pin, LOW);
        digitalWrite(_led3Pin, LOW);
        __nextState++;
        SerialLogger::trace("Led turn: 2/6");
        break;
      case 2:
        digitalWrite(_led1Pin, LOW);
        digitalWrite(_led2Pin, LOW);
        digitalWrite(_led3Pin, LOW);
        __nextState++;
        SerialLogger::trace("Led turn: 3/6");
        break;
      case 3:
        digitalWrite(_led1Pin, LOW);
        digitalWrite(_led2Pin, LOW);
        digitalWrite(_led3Pin, HIGH);
        __nextState++;
        SerialLogger::trace("Led turn: 4/6");
        break;
      case 4:
        digitalWrite(_led1Pin, LOW);
        digitalWrite(_led2Pin, HIGH);
        digitalWrite(_led3Pin, HIGH);
        __nextState++;
        SerialLogger::trace("Led turn: 5/6");
        break;
      case 5:
        digitalWrite(_led1Pin, HIGH);
        digitalWrite(_led2Pin, HIGH);
        digitalWrite(_led3Pin, HIGH);
        __nextState = 0;
        SerialLogger::trace("Led turn: 6/6");
        break;
      default:
        SerialLogger::warn("Unwanted default entered. Led turn: %d/6", __nextState);
    }
    return true; // to repeat the action - false to stop
  });
}

Led3Service::~Led3Service() {
  digitalWrite(kLed1Pin, LOW);
  digitalWrite(kLed2Pin, LOW);
  digitalWrite(kLed3Pin, LOW);
}
