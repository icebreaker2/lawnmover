#include<LowPower.h>
#include <IRremote.h>
#include <arduino-timer.h>

// LEDs
const int LED_INTRA_ROTATION_TIME_DELAY = 100;
const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

auto timer = timer_create_default();
volatile int nextState = 0;

// motor ctrl
const int MOTOR = 7; // no PWM / control with 5v only

// Movement
const int LEFT_FWD_PIN = 13; // is no PWM
const int LEFT_BWD_PIN = 12; // is no PWM
const int LEFT_PWM_PIN = 11; // is PWM
const int RIGHT_PWM_PIN = 10; // is PWM
const int RIGHT_FWD_PIN = 9; // is PWM
const int RIGHT_BWD_PIN = 8; // is no PWM

const int LEFT_FWD_PWM = 128; // maximum
const int LEFT_BWD_PWM = 255; // maximum
const int RIGHT_FWD_PWM = 255; // maximum
const int RIGHT_BWD_PWM = 255; // maximum

// TODO make MovementService
const int MOVEMENT_DURATION = 500; // small delay to make more than an impulse

void stopMovement() {
    Serial.println("stopMovement");
    digitalWrite(LEFT_FWD_PIN, LOW);
    digitalWrite(LEFT_BWD_PIN, LOW);
    digitalWrite(RIGHT_FWD_PIN, LOW);
    digitalWrite(RIGHT_BWD_PIN, LOW);
    delay(500);
}

void turnLeft() {
    Serial.println("left / previous");
    stopMovement();
    analogWrite(LEFT_BWD_PIN, LEFT_BWD_PWM);
    analogWrite(RIGHT_FWD_PIN, RIGHT_FWD_PWM);
    //    digitalWrite(LEFT_BWD, HIGH);
    //    digitalWrite(RIGHT_FWD, HIGH);

    Serial.print("Pin 6: HIGH, ");
    Serial.println("Pin 5: LOW");

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();
}

void turnRight() {
    Serial.println("right / next");
    stopMovement();
    analogWrite(LEFT_FWD_PIN, LEFT_FWD_PWM);
    analogWrite(RIGHT_BWD_PIN, RIGHT_BWD_PWM);
    //    digitalWrite(LEFT_FWD, HIGH);
    //    digitalWrite(RIGHT_BWD, HIGH);

    Serial.print("Pin 6: LOW, ");
    Serial.println("Pin 5: HIGH");

    //    delay(MOVEMENT_DURATION);
    //    stopMovement();
}

void moveForward() {
    Serial.println("forward / up");
    stopMovement();
    analogWrite(LEFT_FWD_PIN, LEFT_FWD_PWM);
    analogWrite(RIGHT_FWD_PIN, RIGHT_FWD_PWM);
    //    digitalWrite(LEFT_FWD, HIGH);
    //    digitalWrite(RIGHT_FWD, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();
}

void moveBackward() {
    Serial.println("backwards / down");
    stopMovement();
    analogWrite(LEFT_BWD_PIN, LEFT_BWD_PWM);
    analogWrite(RIGHT_BWD_PIN, RIGHT_BWD_PWM);
    //digitalWrite(LEFT_BWD, HIGH);
    //digitalWrite(RIGHT_BWD, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();
}

// IR remote commands
const int IR_COMMAND_RECV = A4; // 

boolean rawSpinCheck() {
    if (IrReceiver.decodedIRData.rawDataPtr->rawlen == 4) {
        const int firstByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[0];
        const int secondByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[1];
        const int thirdByte = IrReceiver.decodedIRData.rawDataPtr->rawbuf[2];
        const int fourthByte =  IrReceiver.decodedIRData.rawDataPtr->rawbuf[3];
        if (firstByte >= 1870 && firstByte <= 2050 && secondByte >= 177 && secondByte <= 182 && thirdByte >= 43 && thirdByte <= 47 && fourthByte >= 10 && fourthByte <= 13) {
            // observed in various tests
            spinMotor();
            return true;
        }
    }
    return false;
}


// Debug
const int DEBUG_PIN_1 = 2;
const int DEBUG_PIN_2 = 4;

void setup() {
    // sr debug
    Serial.begin(9600);

    // LEDs (all on)
    pinMode(LED_BUNDLE_1, OUTPUT);
    pinMode(LED_BUNDLE_2, OUTPUT);
    pinMode(LED_BUNDLE_3, OUTPUT);
    digitalWrite(LED_BUNDLE_1, HIGH);
    digitalWrite(LED_BUNDLE_2, HIGH);
    digitalWrite(LED_BUNDLE_3, HIGH);
    timer.every(LED_INTRA_ROTATION_TIME_DELAY, [](void*) -> bool {
        //        switch (nextState) {
        //            case 0:
        //                digitalWrite(LED_BUNDLE_1, HIGH);
        //                digitalWrite(LED_BUNDLE_2, HIGH);
        //                digitalWrite(LED_BUNDLE_3, LOW);
        //                nextState++;
        //                Serial.println("1/6");
        //                break;
        //            case 1:
        //                digitalWrite(LED_BUNDLE_1, HIGH);
        //                digitalWrite(LED_BUNDLE_2, LOW);
        //                digitalWrite(LED_BUNDLE_3, LOW);
        //                nextState++;
        //                Serial.println("2/6");
        //                break;
        //            case 2:
        //                digitalWrite(LED_BUNDLE_1, LOW);
        //                digitalWrite(LED_BUNDLE_2, LOW);
        //                digitalWrite(LED_BUNDLE_3, LOW);
        //                nextState++;
        //                Serial.println("3/6");
        //                break;
        //            case 3:
        //                digitalWrite(LED_BUNDLE_1, LOW);
        //                digitalWrite(LED_BUNDLE_2, LOW);
        //                digitalWrite(LED_BUNDLE_3, HIGH);
        //                nextState++;
        //                Serial.println("4/6");
        //                break;
        //            case 4:
        //                digitalWrite(LED_BUNDLE_1, LOW);
        //                digitalWrite(LED_BUNDLE_2, HIGH);
        //                digitalWrite(LED_BUNDLE_3, HIGH);
        //                nextState++;
        //                Serial.println("5/6");
        //                break;
        //            case 5:
        //                digitalWrite(LED_BUNDLE_1, HIGH);
        //                digitalWrite(LED_BUNDLE_2, HIGH);
        //                digitalWrite(LED_BUNDLE_3, HIGH);
        //                nextState = 0;
        //                Serial.println("6/6");
        //                break;
        //        }
        return true; // to repeat the action - false to stop
    });

    timer.every(MOTOR_SPIN_CHECK_TIME_DELAY, [](void*) -> bool {
        //        Serial.print("MotorCtr after ");
        //        Serial.print(MOTOR_SPIN_CHECK_TIME_DELAY);
        //        Serial.print(" ms was: ");
        //        Serial.print(motorSpinCmdReceived);
        //        Serial.print("/");
        //        Serial.println(MOTOR_SPIN_CHECK_THRESHOLD);
        if (motorSpinCmdReceived < MOTOR_SPIN_CHECK_THRESHOLD) {
            stopMotor();
        }
        motorSpinCmdReceived = 0; // reset

        return true; // to repeat the action - false to stop
    });

    // Movement (brake)
    //    stopMovement();

    // IR remote commands
    IrReceiver.begin(IR_COMMAND_RECV, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);

    // motor ctrl (no spinning)
    pinMode(MOTOR, OUTPUT);
    digitalWrite(MOTOR, LOW);

    // debug pin always high
    pinMode(DEBUG_PIN_1, OUTPUT);
    digitalWrite(DEBUG_PIN_1, HIGH);
    pinMode(DEBUG_PIN_2, OUTPUT);
    digitalWrite(DEBUG_PIN_2, HIGH);
}

void loop() {
    checkAndApplyIRCommand();

    // tick timers
    auto ticks = timer.tick();

    Serial.print("Pin 10: ");
    Serial.print(digitalRead(LEFT_FWD_PIN));
    Serial.print(", Pin 9: ");
    Serial.print(digitalRead(LEFT_BWD_PIN));
    Serial.print(", Pin 6: ");
    Serial.print(digitalRead(RIGHT_FWD_PIN));
    Serial.print(", Pin 5: ");
    Serial.println(digitalRead(RIGHT_BWD_PIN));
    delay(500);

    //    LEFT_FWD_PWM -= 1;
    //    if (LEFT_FWD_PWM < 0) {
    //      LEFT_FWD_PWM = 255;
    //    }
    analogWrite(LEFT_PWM_PIN, LEFT_FWD_PWM);
    analogWrite(RIGHT_PWM_PIN, RIGHT_FWD_PWM);

    Serial.println("Left FWD PWM: ");
    Serial.println(LEFT_FWD_PWM);
}
