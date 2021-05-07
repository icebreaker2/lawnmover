#include<LowPower.h>
#include <IRremote.h>
#include <arduino-timer.h>

// LEDs
const int LED_INTRA_ROTATION_TIME_DELAY = 100;
const int LED_BUNDLE_1 = 13; // no PWN / control with 5v only
const int LED_BUNDLE_2 = 12; // no PWN // control with 5v only
const int LED_BUNDLE_3 = 11; // is PWM

auto timer = timer_create_default();
volatile int nextState = 0;

// motor ctrl
const int MOTOR = 7; // no PWM / control with 5v only
const int MOTOR_SPIN_CHECK_TIME_DELAY = 700;
const int MOTOR_SPIN_CHECK_THRESHOLD = 5;
volatile int motorSpinCmdReceived = 0;

void startMotor() {
    Serial.println("start motor (ST/REPT)");
    digitalWrite(MOTOR, HIGH);
}

void stopMotor() {
    Serial.println("motor stop / pause");
    digitalWrite(MOTOR, LOW);
}

void spinMotor() {
    motorSpinCmdReceived++;
    Serial.println("motor spin / ST/REPT");
}

// Movement
const int LEFT_FWD = 10; // is PWM
const int LEFT_BWD = 9; // is PWM
const int RIGHT_FWD = 6; // is PWM
const int RIGHT_BWD = 5; // is PWM

// TODO make MovementService
const int MOVEMENT_DURATION = 500; // small delay to make more than an impulse

void stopMovement() {
    digitalWrite(LEFT_FWD, LOW);
    digitalWrite(LEFT_BWD, LOW);
    digitalWrite(RIGHT_FWD, LOW);
    digitalWrite(RIGHT_BWD, LOW);
}

void turnLeft() {
    Serial.println("left / previous");
    stopMovement();
    digitalWrite(LEFT_BWD, HIGH);
    digitalWrite(RIGHT_FWD, HIGH);

    delay(MOVEMENT_DURATION);
    stopMovement();
}

void turnRight() {
    Serial.println("right / next");
    stopMovement();
    digitalWrite(LEFT_FWD, HIGH);
    digitalWrite(RIGHT_BWD, HIGH);

    delay(MOVEMENT_DURATION);
    stopMovement();
}

void moveForward() {
    Serial.println("forward / up");
    stopMovement();
    digitalWrite(RIGHT_FWD, HIGH);
    digitalWrite(RIGHT_FWD, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovement();
}

void moveBackward() {
    Serial.println(" backwards / down");
    stopMovement();
    digitalWrite(LEFT_BWD, HIGH);
    digitalWrite(RIGHT_BWD, HIGH);

    //delay(MOVEMENT_DURATION);
    //stopMovementop();
}


// IR remote commands
const int IR_COMMAND_RECV = 8; // no PWM / control with 5v onl

void checkAndApplyIRCommand() {
    if (IrReceiver.decode()) {
        /*
              !!!Important!!! Enable receiving of the next value,
              since receiving has stopped after the end of the current received data packet.
        */
        IrReceiver.resume(); // Enable receiving of the next value

        boolean repeat = IrReceiver.decodedIRData.decodedRawData == 0;
        if (repeat) {
            if (IrReceiver.decodedIRData.command == 0xD) {
                // motor on / play and pause
                spinMotor();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // motor stop / play and pause
                stopMotor();
            } else {
                //Serial.println("Unknown repeat command");
            }
        } else {
            // Finally, check the received data and perform actions according to the received command/
            if (IrReceiver.decodedIRData.command == 0x44) {
                // left / previous
                turnLeft();
            } else if (IrReceiver.decodedIRData.command == 0x43) {
                // right / next
                turnRight();
            } else if (IrReceiver.decodedIRData.command == 0x9) {
                // forward / up
                moveForward();
            } else if (IrReceiver.decodedIRData.command == 0x7) {
                // backwards / down
                moveBackward();
            } else if (IrReceiver.decodedIRData.command == 0x47) {
                // Func / stop (Movement)
                Serial.println(" Func / stop");
                stopMovement();
            } else if (IrReceiver.decodedIRData.command == 0x40) {
                // motor stop / play and pause
                stopMotor();
            } else if (IrReceiver.decodedIRData.command == 0xD) {
                startMotor();
            } else {
                //Serial.println("Unknown single command");
                // Print a short summary of received data
                IrReceiver.printIRResultShort(&Serial);
                if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                    // We have an unknown protocol here, print more info
                    IrReceiver.printIRResultRawFormatted(&Serial, true);
                }
            }
        }
    }
}

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
        switch (nextState) {
            case 0:
                digitalWrite(LED_BUNDLE_1, HIGH);
                digitalWrite(LED_BUNDLE_2, HIGH);
                digitalWrite(LED_BUNDLE_3, LOW);
                nextState++;
                //Serial.println("1/6");
                break;
            case 1:
                digitalWrite(LED_BUNDLE_1, HIGH);
                digitalWrite(LED_BUNDLE_2, LOW);
                digitalWrite(LED_BUNDLE_3, LOW);
                nextState++;
                //Serial.println("2/6");
                break;
            case 2:
                digitalWrite(LED_BUNDLE_1, LOW);
                digitalWrite(LED_BUNDLE_2, LOW);
                digitalWrite(LED_BUNDLE_3, LOW);
                nextState++;
                //Serial.println("3/6");
                break;
            case 3:
                digitalWrite(LED_BUNDLE_1, LOW);
                digitalWrite(LED_BUNDLE_2, LOW);
                digitalWrite(LED_BUNDLE_3, HIGH);
                nextState++;
                //Serial.println("4/6");
                break;
            case 4:
                digitalWrite(LED_BUNDLE_1, LOW);
                digitalWrite(LED_BUNDLE_2, HIGH);
                digitalWrite(LED_BUNDLE_3, HIGH);
                nextState++;
                //Serial.println("5/6");
                break;
            case 5:
                digitalWrite(LED_BUNDLE_1, HIGH);
                digitalWrite(LED_BUNDLE_2, HIGH);
                digitalWrite(LED_BUNDLE_3, HIGH);
                nextState = 0;
                //Serial.println("6/6");
                break;
        }
        return true; // to repeat the action - false to stop
    });

    timer.every(MOTOR_SPIN_CHECK_TIME_DELAY, [](void*) -> bool {
        Serial.print("MotorCtr after ");
        Serial.print(MOTOR_SPIN_CHECK_TIME_DELAY);
        Serial.print(" ms was: ");
        Serial.print(motorSpinCmdReceived);
        Serial.print("/");
        Serial.println(MOTOR_SPIN_CHECK_THRESHOLD);
        if (motorSpinCmdReceived < MOTOR_SPIN_CHECK_THRESHOLD) {
            stopMotor();
        }
        motorSpinCmdReceived = 0; // reset
        
        return true; // to repeat the action - false to stop
    });

    // Movement (brake)
    pinMode(LEFT_FWD, OUTPUT);
    pinMode(LEFT_BWD, OUTPUT);
    pinMode(RIGHT_FWD, OUTPUT);
    pinMode(RIGHT_BWD, OUTPUT);
    digitalWrite(LEFT_FWD, LOW);
    digitalWrite(LEFT_BWD, LOW);
    digitalWrite(RIGHT_FWD, LOW);
    digitalWrite(RIGHT_BWD, LOW);

    // IR remote commands
    IrReceiver.begin(IR_COMMAND_RECV, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);

    // motor ctrl (no spinning)
    pinMode(MOTOR, OUTPUT);
    digitalWrite(MOTOR, LOW);
}

void loop() {
    checkAndApplyIRCommand();

    // tick timers
    auto ticks = timer.tick();
    //Serial.println(ticks);
}
