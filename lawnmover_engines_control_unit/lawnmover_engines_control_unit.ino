#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_commands.h>

#include "spi_slave.h"
#include "watchdog.h"

#include "mover.h"
#include "motor.h"

const int LEFT_FWD_PIN = 8; // is PWM
const int LEFT_BWD_PIN = 9; // is no PWM
const int LEFT_PWM_PIN = 6; // is PWM
const int RIGHT_PWM_PIN = 5; // is PWM
const int RIGHT_FWD_PIN = 4; // is no PWM
const int RIGHT_BWD_PIN = 7; // is no PWM

const int MOTOR_PIN = 3; // is PWM / control with 5v only

const int SCK_PIN_ORANGE   = 13; // D13 = pin19 = PortB.5
const int MISO_PIN_YELLOW  = 12; // D12 = pin18 = PortB.4
const int MOSI_PIN_GREEN  = 11; // D11 = pin17 = PortB.3
const int SS_PIN_BLUE    = 10; // D10 = pin16 = PortB.2

const int EXPECTED_SPI_COMMANDS_PER_SECONDS = 3;


// Debug
const int DEBUG_PIN = 2; // is no PWM

auto _timer = timer_create_default();

const int motor_spin_set_interval = 100;
MotorService *_motorService;
const int steering_set_interval = 100;
MoverService *_moverService;

const int k_watchdog_validation_interval = 1000;
const int k_watchdog_valid_threshold = ENGINE_COMMANDS *  EXPECTED_SPI_COMMANDS_PER_SECONDS;
Watchdog *_watchdog = Watchdog::getFromScheduled(k_watchdog_validation_interval, k_watchdog_valid_threshold, [](void) -> bool {
       _moverService->set_left_wheels_power(LEFT_WHEEL_STEERING_COMMAND, 0);
       _moverService->set_right_wheels_power(RIGHT_WHEEL_STEERING_COMMAND, 0);
       _motorService->set_rotation_speed(MOTOR_SPEED_COMMAND, 0);
    }, _timer);

int k_amount_spi_commands = 3;
bool (*spi_commands[])(int16_t, int16_t) = {
    [](int16_t id, int16_t wheelsPower) -> bool {
        _watchdog->incrementCounter();
        return _moverService->set_left_wheels_power(id, wheelsPower);
    },
    [](int16_t id, int16_t wheelsPower) -> bool {
        _watchdog->incrementCounter();
        return _moverService->set_right_wheels_power(id, wheelsPower);
    },
    [](int16_t id, int16_t rotation_speed) -> bool {
        _watchdog->incrementCounter();
        return _motorService->set_rotation_speed(id, rotation_speed);
    }};

void setup() {
    SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
    // TODO make static object to ease dynamic memory usage
    _motorService = new MotorService(MOTOR_PIN, MOTOR_SPEED_COMMAND);
    _motorService->printInit();
    // TODO make static object to ease dynamic memory usage
    _moverService = new MoverService(LEFT_FWD_PIN, LEFT_BWD_PIN, LEFT_PWM_PIN, RIGHT_PWM_PIN, RIGHT_FWD_PIN,
                                     RIGHT_BWD_PIN, LEFT_WHEEL_STEERING_COMMAND, RIGHT_WHEEL_STEERING_COMMAND);
    _moverService->printInit();
    SpiSlave spiSlave(SCK_PIN_ORANGE, MISO_PIN_YELLOW, MOSI_PIN_GREEN, SS_PIN_BLUE, spi_commands, k_amount_spi_commands, ENGINE_COMMANDS * COMMAND_FRAME_SIZE);

    _timer.every(motor_spin_set_interval, [](void*) -> bool {
        _motorService->spinMotor();
        return true; // to repeat the action - false to stop
    });

    _timer.every(steering_set_interval, [](void*) -> bool {
        _moverService->interpret_state();
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
