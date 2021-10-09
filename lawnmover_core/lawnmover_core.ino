#include <arduino_timer_uno.h>
#include <SerialLogger.h>

#include "Mover.h"
#include "Motor.h"
#include "LED.h"

const int LEFT_FWD_PIN = 9; // is PWM
const int LEFT_BWD_PIN = 8; // is no PWM
const int LEFT_PWM_PIN = 3; // is PWM
const int RIGHT_PWM_PIN = 10; // is PWM
const int RIGHT_FWD_PIN = 5; // is PWM
const int RIGHT_BWD_PIN = 4; // is no PWM

const int LEFT_FWD_PWM = 255; // maximum
const int LEFT_BWD_PWM = 255; // maximum
const int RIGHT_FWD_PWM = 255; // maximum
const int RIGHT_BWD_PWM = 255; // maximum

const int MOTOR_PIN = 2; // no PWM / control with 5v only

const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

// Debug
//const int DEBUG_PIN_1 = 2;
//const int DEBUG_PIN_2 = 3;
//const int DEBUG_PIN_3 = 4;

auto _timer = timer_create_default();
MoverService *_moverService;
MotorService *_motorService;

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
    _motorService = new MotorService(MOTOR_PIN, _timer, false);
    _motorService->printInit();
    _moverService = new MoverService(LEFT_FWD_PIN, LEFT_BWD_PIN, LEFT_PWM_PIN, RIGHT_PWM_PIN, RIGHT_FWD_PIN,
                                     RIGHT_BWD_PIN, LEFT_FWD_PWM, LEFT_BWD_PWM, RIGHT_FWD_PWM, RIGHT_BWD_PWM);
    _moverService->printInit();
    Led3Service _ledService(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);

    // debug pin always high
    //    pinMode(DEBUG_PIN_1, OUTPUT);
    //    digitalWrite(DEBUG_PIN_1, HIGH);
    //    pinMode(DEBUG_PIN_2, OUTPUT);
    //    digitalWrite(DEBUG_PIN_2, HIGH);
    //    pinMode(DEBUG_PIN_3, OUTPUT);
    //    digitalWrite(DEBUG_PIN_3, HIGH);

    // TODO enable dead mens button check again

    // DEBUG START
    //    _moverService->moveForward();
    //    _moverService->moveBackward();
    //    _moverService->turnRight();
    //    _moverService->turnLeft();

    //    _motorService->startMotor();
    // DEBUG END
}

void loop() {
    // DEBUG START
    //        _moverService->changeLeftPwmRate(255);
    //        _moverService->changeRightPwmRate(255);
    //        SerialLogger::debug("change to 255");
    //        delay(1500);
    //
    //        _moverService->changeLeftPwmRate(128);
    //        _moverService->changeRightPwmRate(128);
    //        SerialLogger::debug("change to 128");
    //        delay(1500);
    //
    //        _moverService->changeLeftPwmRate(56);
    //        _moverService->changeRightPwmRate(56);
    //        SerialLogger::debug("change to 56");
    //        delay(1500);
    //
    //        _moverService->changeLeftPwmRate(28);
    //        _moverService->changeRightPwmRate(28);
    //        SerialLogger::debug("change to 28");
    //        delay(2500);
    //
    //        _moverService->changeLeftPwmRate(0);
    //        _moverService->changeRightPwmRate(0);
    //        SerialLogger::debug("change to 0");
    //        delay(1000);

    _moverService->moveForward();
    delay(10);
    _moverService->turnRight();
    delay(10);
    _moverService->turnLeft();
    delay(10);
    _moverService->moveBackward();
    delay(10);
    // DEBUG END

    // tick timers
    auto ticks = _timer.tick();
}
