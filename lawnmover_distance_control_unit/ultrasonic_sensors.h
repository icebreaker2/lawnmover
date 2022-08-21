#ifndef ULTRASONIC_SENSORS_H
#define ULTRASONIC_SENSORS_H

#include <Arduino.h>
#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_commands.h>

#define SENSORING_FREQUENCY_DELAY 45
#define MAX_ARDUINO_PINS 13
#define ULTRASONIC_CM_PER_MICROSECOND_AIR 29
#define NO_ECHO_DISTANCE 7.5f

class UltrasonicSensor {
public:
	UltrasonicSensor(const int16_t id, const int txPin, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
			k_id(id), k_txPin(txPin), k_rxPin(rxPin), k_pulseMaxTimeoutMicroSeconds(pulseMaxTimeoutMicroSeconds),
			k_maxDistance((k_pulseMaxTimeoutMicroSeconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2.0f) {
		SerialLogger::debug("Initiating ultrasonic sensor on rxPin %d, with txPin %d and max possible distance at %f",
							k_rxPin, k_txPin, k_maxDistance);
		pinMode(k_txPin, OUTPUT);
		digitalWrite(k_txPin, LOW);
		pinMode(k_rxPin, INPUT);
		_latestDistance = k_maxDistance;
	}

	// having const values is more valuable than this copy-assignment; if you need to move use (smart) pointers
	UltrasonicSensor &UltrasonicSensor::operator=(const UltrasonicSensor &ultrasonicSensor) = delete;

	~UltrasonicSensor() {
		digitalWrite(k_txPin, LOW);
	}

	int getRxPin() const {
		return k_rxPin;
	};

	int16_t getId() const {
		return k_id;
	};

	float getLatestDistance() const {
		return _latestDistance;
	};

	static void triggerTx(const int txPin);

	void updateLatestDistanceWithTx() {
		triggerTx(k_txPin);
		updateLatestDistanceWithoutTx();
	};

	void updateLatestDistanceWithoutTx() {
		long duration_microseconds = pulseIn(k_rxPin, HIGH, k_pulseMaxTimeoutMicroSeconds);
		if (duration_microseconds == 0) {
			// no echo read before timeout
			duration_microseconds = k_pulseMaxTimeoutMicroSeconds;
		}
		// The signal went back and forth but we do only need one distance
		const float new_distance = (duration_microseconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2.0f;
		// SerialLogger::debug(F("new %f vs. old %f (q1: %d, q2: %d)"), new_distance, _latestDistance,
		//                    _latestDistance < NO_ECHO_DISTANCE, _latestDistance >= k_maxDistance);
		if (_latestDistance < NO_ECHO_DISTANCE && new_distance >= k_maxDistance) {
			// The object got closer to the robot and is now probably "at" the robot leading to no echo due to the object. Setting to zero!
			_latestDistance = 0.0f;
			SerialLogger::debug(F("Setting distance of sensor at pin %d to zero"), k_rxPin);
		} else {
			_latestDistance = (_latestDistance + new_distance) / 2.0f;
		}
	};

private:
	const int k_txPin;
	const int k_rxPin;
	const int16_t k_id;
	const int k_pulseMaxTimeoutMicroSeconds;
	const float k_maxDistance;
	// Note: Assuming there is one writer and multiple readers, i. e. volatile is enough
	volatile float _latestDistance;
};

class UltrasonicSensors {
public:

	static UltrasonicSensors *getFromScheduled(const int txPin, const int rxPins[], const int16_t ids[],
											   const int amountSensors, const int pulseMaxTimeoutMicroSeconds,
											   Timer<> &timer,
											   const int sensoring_frequency_delay = SENSORING_FREQUENCY_DELAY);

	UltrasonicSensors(const int txPin, const int *rxPins, const int16_t ids[], const int amountSensors,
					  const int pulseMaxTimeoutMicroSeconds);

	~UltrasonicSensors();

	int getAmountSensors() const {
		return k_amountSensors;
	};

	void addStatusPrinting(Timer<> &timer, const int frequency) const {
		timer.every(frequency, [](UltrasonicSensors *ultrasonicSensors) -> bool {
			if (ultrasonicSensors == nullptr) {
				SerialLogger::error(F("UltrasonicSensor is nullptr. Something wrong, stopping timer iteration"));
				return false;
			} else {
				SerialLogger::info(F("F: %f, FR: %f, FL: %f, RR: %f, RL: %f"),
								   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_COMMAND),
								   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_RIGHT_COMMAND),
								   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_LEFT_COMMAND),
								   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_BACK_RIGHT_COMMAND),
								   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_BACK_LEFT_COMMAND));
				return true; // to repeat the action - false to stop
			}
		}, this);
	};

	void updateNextDistanceFromSensors();

	void updateDistanceFromSensors();

	float getLatestDistanceFromSensorByPin(const int sensorPin) const;

	float getLatestDistanceFromSensorById(const int16_t id) const;

private:
	const int k_txPin;
	const int k_amountSensors;

	UltrasonicSensor **_ultrasonicSensors;
	volatile int _nextSensorIndex = 0;

	int _registeredSensors = 0;
};

#endif // ULTRASONIC_SENSORS_H
