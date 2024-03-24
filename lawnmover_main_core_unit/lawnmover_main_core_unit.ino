#include <serial_logger.h>
#include <robo_pilot.h>

#include "ESP32_PS4_Controller.h"
#include "esp32_spi_master.h"
#include "engine_slave.h"
#include "navigation_slave.h"

// General SPI settings
const int MOSI_PIN_GREEN = 23;
const int MISO_PIN_YELLOW = 19;
const int SCK_PIN_ORANGE = 18;
const long frequency = 2000000;
const long clock_divide = SPI_CLOCK_DIV8;

// Engine SPI slave settings
const int ENGINE_CONTROL_SS_PIN_BLUE = 5;
const int ENGINE_RESTART_PIN_PIN = 13;

// Object Detection SPI slave settings
const int NAVIGATION_CONTROL_SS_PIN_BROWN = 12;
const int NAVIGATION_RESTART_PIN_PIN = 14;

const int spi_schedule_next_slave_commands_interval = 165;

// Create a timer object with default settings: millis resolution, TIMER_MAX_TASKS (=16) task slots, T = void *
Timer<> _timer = timer_create_default();
// General processing + PS4 (Bluetooth) settings
const char *masterMac = "ac:89:95:b8:7f:be";
ESP32_PS4_Controller *esp32Ps4Ctrl = nullptr;
Esp32SpiMaster *esp32_spi_master = nullptr;
const int restart_check_interval = 1000;

RoboPilot *roboPilot = nullptr;
Supervisor *supervisor = nullptr;
const std::vector<DirectionDistance::Direction> directions = DirectionDistance::create_default_directions();

void re_setup_spi_communication() {
	// delete the master will tear down all slaves (inclusive their power supply) put into master
	SerialLogger::info(F("Shutting down all previous slaves"));
	delete esp32_spi_master;
	SerialLogger::info(F("(Re)Setting up all slaves"));
	esp32_spi_master = new Esp32SpiMaster(SCK_PIN_ORANGE, MISO_PIN_YELLOW, MOSI_PIN_GREEN, frequency);

	const int engine_slave_id = Esp32SpiMaster::take_free_id();
	if (engine_slave_id >= 0) {
		SpiSlaveHandler *spi_slave_handler = esp32_spi_master->get_handler(ENGINE_CONTROL_SS_PIN_BLUE);
		if (spi_slave_handler == nullptr) {
			SerialLogger::error(F("Cannot add a new engine slave. Failed to set up a new spi slave handler"));
			Esp32SpiMaster::put_free_id(engine_slave_id);
		} else {
			EngineSlave *spi_slave = new EngineSlave(spi_slave_handler, engine_slave_id, ENGINE_CONTROL_SS_PIN_BLUE,
			                                         ENGINE_RESTART_PIN_PIN, supervisor);
			esp32_spi_master->put_slave(spi_slave);
		}
	} else {
		SerialLogger::error(F("Cannot add a new engine slave to. Got no free id from Esp32SpiMaster"));
	}

	const int navigation_slave_id = Esp32SpiMaster::take_free_id();
	if (navigation_slave_id >= 0) {
		SpiSlaveHandler *spi_slave_handler = esp32_spi_master->get_handler(NAVIGATION_CONTROL_SS_PIN_BROWN);
		if (spi_slave_handler == nullptr) {
			SerialLogger::error(F("Cannot add a new navigation slave. Failed to set up a new spi slave handler"));
			Esp32SpiMaster::put_free_id(navigation_slave_id);
		} else {
			NavigationSlave *spi_slave = new NavigationSlave(spi_slave_handler, navigation_slave_id,
			                                                               NAVIGATION_CONTROL_SS_PIN_BROWN,
			                                                               NAVIGATION_RESTART_PIN_PIN, roboPilot);
			esp32_spi_master->put_slave(spi_slave);
		}
	} else {
		SerialLogger::error(F("Cannot add a new navigation slave to. Got no free id from Esp32SpiMaster"));
	}
	esp32_spi_master->schedule(spi_schedule_next_slave_commands_interval, _timer);
}

void setup() {
	SerialLogger::init(9600, SerialLogger::LOG_LEVEL::DEBUG);
	esp32Ps4Ctrl = new ESP32_PS4_Controller(masterMac, _timer);

	roboPilot = new RuleBasedMotionStateRoboPilot(directions);
	supervisor = new Supervisor(esp32Ps4Ctrl, roboPilot);

	_timer.every(restart_check_interval, [](void *) -> bool {
		if (esp32_spi_master == nullptr || esp32_spi_master->stopped()) {
			re_setup_spi_communication();
		}
		return true;
	});
}


void loop() {
	// tick timers
	_timer.tick<void>();
}
