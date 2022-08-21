#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <arduino_timer_uno.h>
#include <serial_logger.h>

class Watchdog {
public:

	static Watchdog *getFromScheduled(const int validation_interval, const int valid_threshold,
									  void (*safety_backup_routine)(), Timer<> &timer);

	Watchdog(const int validation_interval, const int valid_threshold, void (*safety_backup_routine)()) :
			k_validation_interval(validation_interval), k_valid_threshold(valid_threshold), _watchdog_counter(0) {
		_safety_backup_routine = safety_backup_routine;
	}

	~Watchdog() {
		// nothing to do...
	};

	bool validate() const {
		if (_watchdog_counter < k_valid_threshold) {
			SerialLogger::error(F("This is Watchdog. Did not receive enough commands (%d/%d) for some time. Stopping "
								  "all engines"), _watchdog_counter, k_valid_threshold);
			return false;
		} else {
			SerialLogger::debug(F("This is the watchdog. Everything normal. Received enough commands (%d/%d)."),
								_watchdog_counter, k_valid_threshold);
			return true;
		}
	}

	void reset() {
		_watchdog_counter = 0;
	}

	void execSafetyProcedure() const {
		(*_safety_backup_routine)();
	}

	void incrementCounter() {
		_watchdog_counter++;
	};

	int getValidationInterval() const { return k_validation_interval; };

	int getValidationThreshold() const { return k_valid_threshold; };

private:
	const int k_validation_interval;
	const int k_valid_threshold;

	void (*_safety_backup_routine)();

	volatile int _watchdog_counter;
};

#endif // WATCHDOG_H
