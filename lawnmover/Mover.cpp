#include "Mover.h"

MoverService::MoverService(const int leftFwdPin, const int leftBwdPin, const int leftPwmPin, const int rightPwmPin, const int rightFwdPin,
                           const int rightBwdPin, const int leftFwdPwm, const int leftBwdPwm, const int rightFwdPwm, const int rightBwdPwm,
                           const bool debug) :
    kLeftFwdPin(leftFwdPin), kLeftBwdPin(leftBwdPin), kLeftPwmPin(leftPwmPin), kRightPwmPin(rightPwmPin),
    kRightFwdPin(rightFwdPin), kRightBwdPin(rightBwdPin), kLeftFwdPwm(leftFwdPwm), kLeftBwdPwm(leftBwdPwm),
    kRightFwdPwm(rightFwdPwm), kRightBwdPwm(rightBwdPwm), kDebug(debug) {

    pinMode(kLeftFwdPin, OUTPUT);
    pinMode(kLeftBwdPin, OUTPUT);
    pinMode(kLeftPwmPin, OUTPUT);
    pinMode(kRightPwmPin, OUTPUT);
    pinMode(kRightFwdPin, OUTPUT);
    pinMode(kRightBwdPin, OUTPUT);

    changeLeftPwmRate(kLeftFwdPwm);
    changeRightPwmRate(kRightFwdPwm);

    printInit();

    // break initially
    // stopMovement();
}

void MoverService::printInit() {
    Serial.print("Set up MoverService with leftFwdPin(");
    Serial.print(kLeftFwdPin);
    Serial.print("), leftBwdPin(");
    Serial.print(kLeftBwdPin);
    Serial.print("), rightFwdPin(");
    Serial.print(kRightFwdPin);
    Serial.print("), rightBwdPin(");
    Serial.print(kRightBwdPin);
    Serial.print("), leftPwmPin(");
    Serial.print(kLeftPwmPin);
    Serial.print("), rightPwmPin(");
    Serial.print(kRightPwmPin);
    Serial.print(") while at ");
    Serial.print(currentLeftPwm);
    Serial.print(" pwm for left and ");
    Serial.print(currentRightPwm);
    Serial.println(" pwm for right.");

    printState();
}

void MoverService::printState() {
    if (kDebug) {
        Serial.print("Pin ");
        Serial.print(kLeftFwdPin);
        Serial.print(": ");
        Serial.print(digitalRead(kLeftFwdPin));
        Serial.print(", Pin ");
        Serial.print(kLeftBwdPin);
        Serial.print(": ");
        Serial.print(digitalRead(kLeftBwdPin));
        Serial.print(", Pin ");
        Serial.print(kRightFwdPin);
        Serial.print(": ");
        Serial.print(digitalRead(kRightFwdPin));
        Serial.print(", Pin ");
        Serial.print(kRightBwdPin);
        Serial.print(": ");
        Serial.println(digitalRead(kRightBwdPin));
    }
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

    printState();
}

void MoverService::turnLeft() {
    stopMovement();
    Serial.println("left / previous");

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
    Serial.println("right / next");

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
    Serial.println("forward / up");

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
    Serial.println("backwards / down");

    changeLeftPwmRate(kLeftBwdPwm);
    changeRightPwmRate(kRightBwdPwm);

    digitalWrite(kLeftBwdPin, HIGH);
    digitalWrite(kRightBwdPin, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();

    printState();
}

void MoverService::changeLeftPwmRate(const int rate) {
    Serial.print("Changing Pin ");
    Serial.print(kLeftPwmPin);
    Serial.print(" to ");
    Serial.println(rate);
    analogWrite(kLeftPwmPin, rate);
    currentLeftPwm = rate;

    printState();
}

void MoverService::changeRightPwmRate(const int rate) {
    Serial.print("Changing Pin ");
    Serial.print(kRightPwmPin);
    Serial.print(" to ");
    Serial.println(rate);
    analogWrite(kRightPwmPin, rate);
    currentRightPwm = rate;

    printState();
}
