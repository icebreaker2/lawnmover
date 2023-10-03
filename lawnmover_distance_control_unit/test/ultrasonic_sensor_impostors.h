#include "../src/ultrasonic_sensors.h"

class UltrasonicSensorImpostor : public UltrasonicSensor {
public:
	UltrasonicSensorImpostor(const int16_t id, const int pulseMaxTimeoutMicroSeconds,
	                         const float *ultrasonicSensorSchedule, const int16_t scheduleEnd) :
			UltrasonicSensor(id, pulseMaxTimeoutMicroSeconds),
			k_ultrasonicSensorSchedule(ultrasonicSensorSchedule), k_scheduleEnd(scheduleEnd) {
		// nothing to do here...
	};

	~UltrasonicSensorImpostor() {
		delete k_ultrasonicSensorSchedule;
	};

	virtual void updateLatestDistanceWithoutTx();

private:
	const float *k_ultrasonicSensorSchedule;
	const int16_t k_scheduleEnd;
	int _iterator = 0;
};

void UltrasonicSensorImpostor::updateLatestDistanceWithoutTx() {
	const float new_distance = k_ultrasonicSensorSchedule[_iterator];
	SerialLogger::info(F("Iterator: %d, distance: %f, sensor: %d"), _iterator, new_distance, getId());
	updateDistance(new_distance);
	_iterator = ++_iterator % k_scheduleEnd;
}

class UltrasonicSensorImpostors : public UltrasonicSensors {
public:
	UltrasonicSensorImpostors(const int16_t ids[], const int amountSensors,
	                          const int pulseMaxTimeoutMicroSeconds, const float **ultrasonicSensorsSchedules,
	                          const int16_t scheduleEnd) : UltrasonicSensors(amountSensors),
	                                                       k_ultrasonicSensorsSchedules(ultrasonicSensorsSchedules) {
		_ultrasonicSensors = (UltrasonicSensorImpostor **) malloc(k_amountSensors * sizeof(UltrasonicSensorImpostor));
		for (int i = 0; i < k_amountSensors; i++) {
			_ultrasonicSensors[_registeredSensors++] = new UltrasonicSensorImpostor(ids[i], pulseMaxTimeoutMicroSeconds,
			                                                                        k_ultrasonicSensorsSchedules[i],
			                                                                        scheduleEnd);
		}
	};

	~UltrasonicSensorImpostors() {
		// base class destructors called automatically
		delete k_ultrasonicSensorsSchedules;
	};

	static UltrasonicSensorImpostors *getFromScheduled(const int rxPins[], const int16_t ids[],
	                                                   const int amountSensors, const int pulseMaxTimeoutMicroSeconds,
	                                                   Timer<> &timer, const float **ultrasonicSensorsSchedules,
	                                                   const int16_t scheduleEnd);

private:
	const float **k_ultrasonicSensorsSchedules;
};

UltrasonicSensorImpostors *UltrasonicSensorImpostors::getFromScheduled(
		const int rxPins[], const int16_t ids[], const int amountSensors, const int pulseMaxTimeoutMicroSeconds,
		Timer<> &timer, const float **ultrasonicSensorsSchedules, const int16_t scheduleEnd) {
	UltrasonicSensorImpostors *ultrasonicSensors = new UltrasonicSensorImpostors(
			ids, amountSensors, pulseMaxTimeoutMicroSeconds, ultrasonicSensorsSchedules, scheduleEnd);
	UltrasonicSensors::schedule(timer, ultrasonicSensors);
	return ultrasonicSensors;
}
