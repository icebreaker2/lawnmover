#include "watchdog.h"

Watchdog *Watchdog::getFromScheduled(const int validation_interval, const int valid_threshold,
									 void (*safety_backup_routine)(void), Timer<> &timer) {
	Watchdog *watchdog = new Watchdog(validation_interval, valid_threshold, safety_backup_routine);
	timer.every(watchdog->getValidationInterval(), [](Watchdog *watchdog) -> bool {
		if (!watchdog->validate()) {
			watchdog->execSafetyProcedure();
		}
		watchdog->reset();
		return true; // to repeat the action - false to stop
	}, watchdog);
	return watchdog;
}