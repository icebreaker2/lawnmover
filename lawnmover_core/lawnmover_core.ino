#include <arduino_timer_uno.h>
#include <SerialLogger.h>

#include "Mover.h"
#include "Motor.h"
#include "LED.h"
#include "SpiMaster.h"

// To not mix with internally used pins
const int SCLK_SPI_INTERNAL_PIN = 13;
const int MISO_SPI_INTERNAL_PIN = 12;
const int MOSI_SPI_INTERNAL_PIN = 11;
const int DEFAULT_SS_PIN_SPI_INTERNAL_PIN = 10;

const int LEFT_FWD_PIN = 9; // is PWM
const int LEFT_BWD_PIN = 8; // is no PWM
const int LEFT_PWM_PIN = 6; // is PWM
const int RIGHT_PWM_PIN = 5; // is PWM
const int RIGHT_FWD_PIN = 7; // is no PWM
const int RIGHT_BWD_PIN = 4; // is no PWM

const int MOTOR_PIN = 3; // is no PWM / control with 5v only

const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

const int LEFT_FWD_PWM = 255; // maximum
const int LEFT_BWD_PWM = 255; // maximum
const int RIGHT_FWD_PWM = 255; // maximum
const int RIGHT_BWD_PWM = 255; // maximum

const int SPI_REQUEST_FREQENCY = 500;

// Debug
const int DEBUG_PIN = 2;

auto _timer = timer_create_default();
MoverService *_moverService;
MotorService *_motorService;

void ps4_bt_controller_callback(const int slavePin, const int delayPerSpiRead) {
    //Starts communication with Slave connected to master (slave select)
    digitalWrite(slavePin, LOW);

    //Mastereceive = SPI.transfer(Mastersend);
    SerialLogger::info("SPI master send-receive to-from slave connected to pin %d", slavePin);

    char buf [20];
    SPI.transfer (1);   // initiate transmission
    for (int pos = 0; pos < sizeof (buf) - 1; pos++) {
        // TODO can this be lower? Is transfer not blocking already?
        delayMicroseconds (delayPerSpiRead);
        buf [pos] = SPI.transfer (0);
        if (buf [pos] == 0) {
            break;
        }
    }

    buf [sizeof (buf) - 1] = 0;  // ensure terminating null
    SerialLogger::info("SPI master received: %s", buf);

    // disable Slave Select again until next step
    digitalWrite(slavePin, HIGH);
}

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
    _motorService = new MotorService(MOTOR_PIN, _timer, false);
    _motorService->printInit();
    _moverService = new MoverService(LEFT_FWD_PIN, LEFT_BWD_PIN, LEFT_PWM_PIN, RIGHT_PWM_PIN, RIGHT_FWD_PIN,
                                     RIGHT_BWD_PIN, LEFT_FWD_PWM, LEFT_BWD_PWM, RIGHT_FWD_PWM, RIGHT_BWD_PWM);
    _moverService->printInit();
    Led3Service _ledService(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);
    SpiMaster spiMaster;
    spiMaster.addSlave(DEFAULT_SS_PIN_SPI_INTERNAL_PIN, SPI_REQUEST_FREQENCY, _timer, &ps4_bt_controller_callback);

    // debug pin always high
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, HIGH);

    // DEBUG START
    // _moverService->moveForward();
    // _moverService->moveBackward();
    // _moverService->turnRight();
    // _moverService->turnLeft();

    // _motorService->startMotor();
    // DEBUG END
}

void loop() {
    // DEBUG START
    // _moverService->changeLeftPwmRate(255);
    // _moverService->changeRightPwmRate(255);
    // SerialLogger::debug("change to 255");
    // delay(1500);
    //
    // _moverService->changeLeftPwmRate(128);
    // _moverService->changeRightPwmRate(128);
    // SerialLogger::debug("change to 128");
    // delay(1500);
    //
    // _moverService->changeLeftPwmRate(56);
    //  _moverService->changeRightPwmRate(56);
    // SerialLogger::debug("change to 56");
    // delay(1500);
    //
    // _moverService->changeLeftPwmRate(28);
    // _moverService->changeRightPwmRate(28);
    // SerialLogger::debug("change to 28");
    // delay(2500);
    //
    // _moverService->changeLeftPwmRate(0);
    // _moverService->changeRightPwmRate(0);
    // SerialLogger::debug("change to 0");
    // delay(1000);

    //_moverService->moveForward();
    //delay(10);
    //_moverService->turnRight();
    //delay(10);
    //_moverService->turnLeft();
    //delay(10);
    //_moverService->moveBackward();
    //delay(10);
    // DEBUG END

    // tick timers
    auto ticks = _timer.tick();
}
