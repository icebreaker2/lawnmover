#include "mover.h"
#include <serial_logger.h>

MoverService::MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin,
                           const int rightFwdPin, const int rightBwdPin) :
    kLeftFwdPin(leftFwdPin), kLeftBwdPin(leftBwdPin), kLeftPwmPin(leftPwmPin), kRightPwmPin(rightPwmPin),
    kRightFwdPin(rightFwdPin), kRightBwdPin(rightBwdPin) {

    pinMode(kLeftFwdPin, OUTPUT);
    pinMode(kLeftBwdPin, OUTPUT);
    pinMode(kLeftPwmPin, OUTPUT);
    pinMode(kRightPwmPin, OUTPUT);
    pinMode(kRightFwdPin, OUTPUT);
    pinMode(kRightBwdPin, OUTPUT);

    // break initially
    stopMovement();
}

void MoverService::printInit() {
    SerialLogger::info("Set up MoverService with leftFwdPin(%d), leftBwdPin(%d), rightFwdPin(%d), rightBwdPin(%d), "
                       "leftPwmPin(%d), rightPwmPin(%d) with leftPwmPin(%d) and rightPwnPin(%d)",
                       kLeftFwdPin, kLeftBwdPin, kRightFwdPin, kRightBwdPin, kLeftPwmPin, kRightPwmPin,
                       kRightFwdPin, kRightBwdPin);
    printState();
}

void MoverService::printState() {
    const bool leftFwdPinState = digitalRead(kLeftFwdPin);
    const bool rightFwdPinState = digitalRead(kRightFwdPin);
    const bool leftBwdPinState = digitalRead(kLeftBwdPin);
    const bool rightBwdPinState = digitalRead(kRightBwdPin);
    SerialLogger::debug("Pin %d: %d, Pin %d: %d, Pin %d: %d, Pin %d: %d",
                        kLeftFwdPin, leftFwdPinState, kLeftBwdPin, leftBwdPinState, kRightFwdPin, rightFwdPinState,
                        kRightBwdPin, rightBwdPinState);
}

MoverService::~MoverService() {
    stopMovement();
}


void MoverService::stopMovement() {
    SerialLogger::trace("stopping movement");
    digitalWrite(kLeftFwdPin, LOW);
    digitalWrite(kLeftBwdPin, LOW);
    digitalWrite(kRightFwdPin, LOW);
    digitalWrite(kRightBwdPin, LOW);

    analogWrite(kLeftPwmPin, 0);
    analogWrite(kRightPwmPin, 0);

    delay(20);
    printState();
}

void MoverService::changeLeftPwmRate(const int rate) {
    // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    if (rate > 15) {
        // reduce sensitivity around anchor point zero
        SerialLogger::debug("Changing Pwm Pin %i (left) to %d", kLeftPwmPin, rate);
        analogWrite(kLeftPwmPin, rate);
    } else {
        SerialLogger::debug("Not setting left rate. Pwm rate was below threshold (%d/15). Stopping left pwm.", rate);
        analogWrite(kLeftPwmPin, 0);
    }
}

void MoverService::changeRightPwmRate(const int rate) {
    // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    if (rate > 15) {
        // reduce sensitivity around anchor point zero
        SerialLogger::debug("Changing Pwm Pin %i (right) to %d", kRightPwmPin, rate);
        analogWrite(kRightPwmPin, rate);
    } else {
        SerialLogger::debug("Not setting right rate. Pwm rate was below threshold (%d/15). Stopping right pwm.", rate);
        analogWrite(kRightPwmPin, 0);
    }
}

void MoverService::interpret_state() {
    const bool left_forward = left_wheels_power >= 0;
    const bool right_forward = right_wheels_power >= 0;
    const int left_rate = left_wheels_power < 0 ? left_wheels_power  * -1 : left_wheels_power ;
    const int right_rate = right_wheels_power < 0 ? right_wheels_power  * -1 : right_wheels_power ;
    if (left_forward) {
        SerialLogger::debug("Left forwards");
        digitalWrite(kLeftBwdPin, LOW);
        digitalWrite(kLeftFwdPin, HIGH);
    } else {
        SerialLogger::debug("Left backwards");
        digitalWrite(kLeftFwdPin, LOW);
        digitalWrite(kLeftBwdPin, HIGH);
    }

    if (right_forward) {
        SerialLogger::debug("Right forwards");
        digitalWrite(kRightBwdPin, LOW);
        digitalWrite(kRightFwdPin, HIGH);
    } else {
        SerialLogger::debug("Right backwards");
        digitalWrite(kRightFwdPin, LOW);
        digitalWrite(kRightBwdPin, HIGH);
    }

    changeLeftPwmRate(left_rate);
    changeRightPwmRate(right_rate);

    // TODO delete expensive printing....
    printState();
}
