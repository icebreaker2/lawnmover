#include <serial_logger.h>
#include <robo_pilot.h>

#include "ESP32_PS4_Controller.h"
#include "esp32_spi_master.h"
#include "engine_slave.h"
#include "obstacle_detection_Slave.h"

// General SPI settings
const int MOSI_PIN_GREEN = 23;
const int MISO_PIN_YELLOW = 19;
const int SCK_PIN_ORANGE = 18;
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;
const int INTER_TRANSACTION_DELAY_MICROSECONDS = 10;

// Engine SPI slave settings
const int ENGINE_CONTROL_SS_PIN_BLUE = 5;
const int ENGINE_RESTART_PIN_PIN = 13;

// Object Detection SPI slave settings
const int OBSTACLE_DETECTION_CONTROL_SS_PIN_BROWN = 12;
const int OBSTACLE_DETECTION_RESTART_PIN_PIN = 14;

const int spi_schedule_next_slave_commands_intervall = 165;

// General processing + PS4 (Bluetooth) settings
auto _timer = timer_create_default();
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl = nullptr;
Esp32SpiMaster *esp32_spi_master = nullptr;
const int restart_check_intervall = 1000;

RoboPilot *_roboPilot = nullptr;

void re_setup_spi_communication() {
	// delete the master will tear down all slaves (inclusive their power supply) put into master
	SerialLogger::info(F("Shutting down all previous slaves"));
	delete esp32_spi_master;
	SerialLogger::info(F("(Re)Setting up all slaves"));
	esp32_spi_master = new Esp32SpiMaster(SCK_PIN_ORANGE, MISO_PIN_YELLOW, MOSI_PIN_GREEN,
	                                      frequency, INTER_TRANSACTION_DELAY_MICROSECONDS);

	const int engine_slave_id = Esp32SpiMaster::take_free_id();
	if (engine_slave_id >= 0) {
		SpiSlaveHandler *spi_slave_handler = esp32_spi_master->get_handler(ENGINE_CONTROL_SS_PIN_BLUE);
		EngineSlave *spi_slave = new EngineSlave(spi_slave_handler, engine_slave_id, ENGINE_CONTROL_SS_PIN_BLUE,
		                                         ENGINE_RESTART_PIN_PIN, esp32Ps4Ctrl, _roboPilot);
		esp32_spi_master->put_slave(spi_slave);
	} else {
		SerialLogger::error(F("Cannot add a new engine slave to. Got no free id from Esp32SpiMaster"));
	}

	const int obstacle_slave_id = Esp32SpiMaster::take_free_id();
	if (obstacle_slave_id >= 0) {
		SpiSlaveHandler *spi_slave_handler = esp32_spi_master->get_handler(OBSTACLE_DETECTION_CONTROL_SS_PIN_BROWN);
		ObstacleDetectionSlave *spi_slave = new ObstacleDetectionSlave(spi_slave_handler, obstacle_slave_id,
		                                                               OBSTACLE_DETECTION_CONTROL_SS_PIN_BROWN,
		                                                               OBSTACLE_DETECTION_RESTART_PIN_PIN, _roboPilot);
		esp32_spi_master->put_slave(spi_slave);
	} else {
		SerialLogger::error(F("Cannot add a new obstacle detection slave to. Got no free id from Esp32SpiMaster"));
	}
	esp32_spi_master->schedule(spi_schedule_next_slave_commands_intervall, _timer);
}

void setup() {
	SerialLogger::init(9600, SerialLogger::LOG_LEVEL::INFO);
	esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);

	_roboPilot = new RuleBasedMotionStateRoboPilot();

	_timer.every(restart_check_intervall, [](void *) -> bool {
		if (esp32_spi_master == nullptr || esp32_spi_master->stopped()) {
			re_setup_spi_communication();
		}
		return true;
	});
}


void loop() {
	// tick timers
	auto ticks = _timer.tick();
}
