#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_commands.h>
#include "led.h"
#include "ultrasonic_sensors.h"

const int SCK_PIN   = 9; // D13 = pin19 = PortB.5
const int MISO_PIN  = 10; // D12 = pin18 = PortB.4
const int MOSI_PIN  = 11; // D11 = pin17 = PortB.3
const int SS_PIN    = 12; // D10 = pin16 = PortB.2

const int AMOUNT_ULTRA_SENSORS = 5;
const int ULTRA_RX_FRONT = 2;
const int ULTRA_RX_FRONT_RIGHT = 6;
const int ULTRA_RX_FRONT_LEFT = 4;
const int ULTRA_RX_REAR_RIGHT = 5;
const int ULTRA_RX_REAR_LEFT = 3;
const int ULTRA_TX_PIN = 7;
const int PULSE_MAX_TIMEOUT_MICROSECONDS = 6000; // 1m distance
const int DEBUG_PRINT_DISTANCE_DELAY = 500;

const int LED_BUNDLE_1 = A0;
const int LED_BUNDLE_2 = A1;
const int LED_BUNDLE_3 = A2;

auto _timer = timer_create_default();

UltrasonicSensors *_ultrasonicSensors;
Led3Service *_ledService;

void setup() {
  SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);

  // Note: Order matters. We alternate rear and front to reduce risiking receiving the echo of a previous tx if sensoring_frequency_delay was choosen too thin.
  int sensorsList [] = {ULTRA_RX_FRONT_LEFT, ULTRA_RX_REAR_RIGHT, ULTRA_RX_FRONT, ULTRA_RX_REAR_LEFT, ULTRA_RX_FRONT_RIGHT};
  _ultrasonicSensors = UltrasonicSensors::getFromScheduled(ULTRA_TX_PIN, sensorsList, AMOUNT_ULTRA_SENSORS, PULSE_MAX_TIMEOUT_MICROSECONDS, _timer);
  _ledService = new Led3Service(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);

    // Some 
  _timer.every(DEBUG_PRINT_DISTANCE_DELAY, [](void*) -> bool {
    SerialLogger::info("F: %f, FR: %f, FL: %f, RR: %f, RL: %f",
    _ultrasonicSensors->getLatestDistanceFromSensor(ULTRA_RX_FRONT),
    _ultrasonicSensors->getLatestDistanceFromSensor(ULTRA_RX_FRONT_RIGHT),
    _ultrasonicSensors->getLatestDistanceFromSensor(ULTRA_RX_FRONT_LEFT),
    _ultrasonicSensors->getLatestDistanceFromSensor(ULTRA_RX_REAR_RIGHT),
    _ultrasonicSensors->getLatestDistanceFromSensor(ULTRA_RX_REAR_LEFT));
    return true; // to repeat the action - false to stop
  });
}

void loop() {
  // tick timers
  auto ticks = _timer.tick();
}
