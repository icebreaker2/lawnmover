#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_commands.h>

#include "mover.h"
#include "motor.h"
#include "led.h"
#include "spi_slave.h"

// To not mix with internally used pins
const int SCLK_SPI_INTERNAL_PIN = 13;
const int MISO_SPI_INTERNAL_PIN = 12;
const int MOSI_SPI_INTERNAL_PIN = 11;
const int DEFAULT_SS_PIN_SPI_INTERNAL_PIN = 10;

const int LEFT_FWD_PIN = 8; // is PWM
const int LEFT_BWD_PIN = 9; // is no PWM
const int LEFT_PWM_PIN = 6; // is PWM
const int RIGHT_PWM_PIN = 5; // is PWM
const int RIGHT_FWD_PIN = 4; // is no PWM
const int RIGHT_BWD_PIN = 7; // is no PWM

const int MOTOR_PIN = 3; // is PWM / control with 5v only

const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

const int SCK_PIN   = 13; // D13 = pin19 = PortB.5
const int MISO_PIN  = 12; // D12 = pin18 = PortB.4
const int MOSI_PIN  = 11; // D11 = pin17 = PortB.3
const int SS_PIN    = 10; // D10 = pin16 = PortB.2

const int EXPECTED_SPI_COMMANDS_PER_SECONDS = 5;
const int EXPECTED_SPI_COMMANDSERROR_MARGIN = 2 * ENGINE_COMMANDS;
volatile int watchdog_counter_ = 0;

// Debug
const int DEBUG_PIN = 2; // is no PWM

auto _timer = timer_create_default();

const int motor_spin_set_interval = 100;
MotorService *_motorService;
const int steering_set_interval = 100;
MoverService *_moverService;

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    _motorService = new MotorService(MOTOR_PIN);
    _motorService->printInit();
    _moverService = new MoverService(LEFT_FWD_PIN, LEFT_BWD_PIN, LEFT_PWM_PIN, RIGHT_PWM_PIN, RIGHT_FWD_PIN,
                                     RIGHT_BWD_PIN);
    _moverService->printInit();
    Led3Service _ledService(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);
    SpiSlave spiSlave(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN,
    [](int16_t wheelsPower) -> bool {
        watchdog_counter_++;
        return _moverService->set_left_wheels_power(wheelsPower);
    },
    [](int16_t wheelsPower) -> bool {
        watchdog_counter_++;
        return _moverService->set_right_wheels_power(wheelsPower);
    },
    [](int16_t rotation_speed) -> bool {
        watchdog_counter_++;
        return _motorService->set_rotation_speed(rotation_speed);
    });

    _timer.every(motor_spin_set_interval, [](void*) -> bool {
        _motorService->spinMotor();
        return true; // to repeat the action - false to stop
    });

    _timer.every(steering_set_interval, [](void*) -> bool {
        _moverService->interpret_state();
        return true; // to repeat the action - false to stop
    });


    _timer.every(1000, [](void*) -> bool {
        if (watchdog_counter_ < ENGINE_COMMANDS *  EXPECTED_SPI_COMMANDS_PER_SECONDS - EXPECTED_SPI_COMMANDSERROR_MARGIN) {
            SerialLogger::error("This is the watchdog. Did not receive enough commands (%d/%d) for some time. Stopping all engines",
            watchdog_counter_, ENGINE_COMMANDS *  EXPECTED_SPI_COMMANDS_PER_SECONDS - EXPECTED_SPI_COMMANDSERROR_MARGIN);
            _moverService->set_left_wheels_power(0);
            _moverService->set_right_wheels_power(0);
            _motorService->set_rotation_speed(0);
        } else {
            SerialLogger::debug("This is the watchdog. Everything normal.");
        }
        watchdog_counter_ = 0;
        return true; // to repeat the action - false to stop
    });

    spiSlave.addSlavePrinting(_timer, 1000);

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
