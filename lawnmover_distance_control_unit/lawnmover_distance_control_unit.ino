#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_slave.h>

#include "led.h"
#include "ultrasonic_sensors.h"

const int SCK_PIN_ORANGE   = 13; // D13 = pin19 = PortB.5
const int MISO_PIN_YELLOW  = 12; // D12 = pin18 = PortB.4
const int MOSI_PIN_GREEN  = 11; // D11 = pin17 = PortB.3
const int SS_PIN_BLUE    = 10; // D10 = pin16 = PortB.2

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

int k_amount_data_push_commands = 0;
bool (*_data_push_commands[])(int16_t, int16_t) = {};

// TODO add GYRO commands
int k_amount_data_request_commands = 1;

bool (*_data_request_commands[])(int16_t, uint8_t *) = {
		[](int16_t id, uint8_t *value_bytes_buffer) -> bool {
			const float latestDistance = _ultrasonicSensors->getLatestDistanceFromSensorById(id);
			if (latestDistance >= 0.0f) {
				memcpy(value_bytes_buffer, &latestDistance, sizeof(latestDistance));
				return true;
			} else {
				// id not known
				return false;
			}
		}
};

void setup() {
	SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);

    _ledService = new Led3Service(LED_BUNDLE_1, LED_BUNDLE_2, LED_BUNDLE_3, _timer);
    
	// Note: Order matters. We alternate rear and front to reduce risiking receiving the echo of a previous tx if sensoring_frequency_delay was choosen too thin.
	const int sensorsRxPinList[] = {ULTRA_RX_FRONT_LEFT, ULTRA_RX_REAR_RIGHT, ULTRA_RX_FRONT, ULTRA_RX_REAR_LEFT,
									ULTRA_RX_FRONT_RIGHT};
	const int sensorsSpiIdList[] = {OBSTACLE_FRONT_LEFT_COMMAND, OBSTACLE_BACK_RIGHT_COMMAND, OBSTACLE_FRONT_COMMAND,
									OBSTACLE_BACK_LEFT_COMMAND, OBSTACLE_FRONT_RIGHT_COMMAND};
	_ultrasonicSensors = UltrasonicSensors::getFromScheduled(ULTRA_TX_PIN, sensorsRxPinList, sensorsSpiIdList,
															 AMOUNT_ULTRA_SENSORS, PULSE_MAX_TIMEOUT_MICROSECONDS,
															 _timer);

	if (SerialLogger::isBelow(SerialLogger::DEBUG)) {
		_ultrasonicSensors->addStatusPrinting(_timer, DEBUG_PRINT_DISTANCE_DELAY);
	}

	SpiSlave::ISRfromArgs(SCK_PIN_ORANGE, MISO_PIN_YELLOW, MOSI_PIN_GREEN, SS_PIN_BLUE, _data_push_commands,
						  k_amount_data_push_commands, _data_request_commands, k_amount_data_request_commands,
						  OBSTACLE_COMMANDS * COMMAND_FRAME_SIZE + GYRO_COMMANDS * COMMAND_FRAME_SIZE);

    SpiSlave::addDebugSlavePrinting(_timer, 1000);
}

void loop() {
	// tick timers
	auto ticks = _timer.tick();
}
