#include "Mover.h"

MoverService::MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin, const int rightFwdPin,
                           const int rightBwdPin, const int leftFwdPwm, const int leftBwdPwm, const int rightFwdPwm, const int rightBwdPwm) :
    kLeftFwdPin(leftFwdPin), kLeftBwdPin(leftBwdPin), kLeftPwmPin(leftPwmPin), kRightPwmPin(rightPwmPin),
    kRightFwdPin(rightFwdPin), kRightBwdPin(rightBwdPin), kLeftFwdPwm(leftFwdPwm), kLeftBwdPwm(leftBwdPwm),
    kRightFwdPwm(rightFwdPwm), kRightBwdPwm(rightBwdPwm) {

    pinMode(kLeftFwdPin, OUTPUT);
    pinMode(kLeftBwdPin, OUTPUT);
    pinMode(kLeftPwmPin, OUTPUT);
    pinMode(kRightPwmPin, OUTPUT);
    pinMode(kRightFwdPin, OUTPUT);
    pinMode(kRightBwdPin, OUTPUT);

    // break initially
    //    stopMovement();
}

MoverService::~MoverService() {
    stopMovement();
}


void MoverService::stopMovement() {
    Serial.println("stopMovement");
    digitalWrite(kLeftFwdPin, LOW);
    digitalWrite(kLeftBwdPin, LOW);
    digitalWrite(kRightFwdPin, LOW);
    digitalWrite(kRightBwdPin, LOW);
    delay(100);
}

void MoverService::turnLeft() {
    Serial.println("left / previous");
    stopMovement();

    digitalWrite(kLeftBwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();
}

void MoverService::turnRight() {
    Serial.println("right / next");
    stopMovement();

    digitalWrite(kLeftFwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();
}

void MoverService::moveForward() {
    Serial.println("forward / up");
    stopMovement();

    digitalWrite(kLeftFwdPin, HIGH);
    digitalWrite(kRightFwdPin, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();
}

void MoverService::moveBackward() {
    Serial.println("backwards / down");
    stopMovement();
    digitalWrite(kLeftBwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();
}

void MoverService::changeLeftPwmRate(const int rate) {
    analogWrite(kLeftPwmPin, rate);
}

void MoverService::changeRightPwmRate(const int rate) {
    analogWrite(kRightPwmPin, rate);
}
