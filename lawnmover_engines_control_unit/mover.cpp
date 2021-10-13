#include "mover.h"
#include <serial_logger.h>

MoverService::MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin,
                           const int rightFwdPin, const int rightBwdPin, const int leftFwdPwm, const int leftBwdPwm,
                           const int rightFwdPwm, const int rightBwdPwm) :
    kLeftFwdPin(leftFwdPin), kLeftBwdPin(leftBwdPin), kLeftPwmPin(leftPwmPin), kRightPwmPin(rightPwmPin),
    kRightFwdPin(rightFwdPin), kRightBwdPin(rightBwdPin), kLeftFwdPwm(leftFwdPwm), kLeftBwdPwm(leftBwdPwm),
    kRightFwdPwm(rightFwdPwm), kRightBwdPwm(rightBwdPwm) {

    pinMode(kLeftFwdPin, OUTPUT);
    pinMode(kLeftBwdPin, OUTPUT);
    pinMode(kLeftPwmPin, OUTPUT);
    pinMode(kRightPwmPin, OUTPUT);
    pinMode(kRightFwdPin, OUTPUT);
    pinMode(kRightBwdPin, OUTPUT);

    changeLeftPwmRate(kLeftFwdPwm);
    changeRightPwmRate(kRightFwdPwm);

    // break initially
    stopMovement();
}

void MoverService::printInit() {
    SerialLogger::info("Set up MoverService with leftFwdPin(%d), leftBwdPin(%d), rightFwdPin(%d), rightBwdPin(%d), "
                       "leftPwmPin(%d), rightPwmPin(%d) while at %d pwm for left and %d pwm for right.",
                       kLeftFwdPin, kLeftBwdPin, kRightFwdPin, kRightBwdPin, kLeftPwmPin, kRightPwmPin, currentLeftPwm,
                       currentRightPwm);
    printState();
}

void MoverService::printState() {
    const bool leftFwPinState = digitalRead(kLeftFwdPin);
    const bool rightFwPinState = digitalRead(kRightFwdPin);
    const bool leftBwPinState = digitalRead(kLeftBwdPin);
    const bool rightBwPinState = digitalRead(kRightBwdPin);
    SerialLogger::debug("Pin %d: %d, Pin %d: %d, Pin %d: %d, Pin %d: %d", kLeftFwdPin, leftFwPinState, kLeftBwdPin,
                        leftBwPinState, kRightFwdPin, rightFwPinState, kRightBwdPin, rightBwPinState);
}

MoverService::~MoverService() {
    stopMovement();
}


void MoverService::stopMovement() {
    SerialLogger::debug("stopMovement");
    digitalWrite(kLeftFwdPin, LOW);
    digitalWrite(kLeftBwdPin, LOW);
    digitalWrite(kRightFwdPin, LOW);
    digitalWrite(kRightBwdPin, LOW);
    delay(100);

    printState();
}

void MoverService::turnLeft() {
    stopMovement();
    SerialLogger::debug("left / previous");

    changeLeftPwmRate(kLeftBwdPwm);
    changeRightPwmRate(kRightFwdPwm);

    digitalWrite(kLeftBwdPin, HIGH);
    digitalWrite(kRightFwdPin, HIGH);

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();

    printState();
}

void MoverService::turnRight() {
    stopMovement();
    SerialLogger::debug("right / next");

    changeLeftPwmRate(kLeftFwdPwm);
    changeRightPwmRate(kRightBwdPwm);

    digitalWrite(kLeftFwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();

    printState();
}

void MoverService::moveForward() {
    stopMovement();
    SerialLogger::debug("forward / up");

    changeLeftPwmRate(kLeftFwdPwm);
    changeRightPwmRate(kRightFwdPwm);

    digitalWrite(kLeftFwdPin, HIGH);
    digitalWrite(kRightFwdPin, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();

    printState();
}

void MoverService::moveBackward() {
    stopMovement();
    SerialLogger::debug("backwards / down");

    changeLeftPwmRate(kLeftBwdPwm);
    changeRightPwmRate(kRightBwdPwm);

    digitalWrite(kLeftBwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();

    printState();
}

void MoverService::changeLeftPwmRate(const int rate) {
    SerialLogger::debug("Changing Pin %i to %d", kLeftPwmPin, rate);
    analogWrite(kLeftPwmPin, rate);
    currentLeftPwm = rate;

    printState();
}

void MoverService::changeRightPwmRate(const int rate) {
    SerialLogger::debug("Changing Pin %i to %d", kRightPwmPin, rate);
    analogWrite(kRightPwmPin, rate);
    currentRightPwm = rate;

    printState();
}
