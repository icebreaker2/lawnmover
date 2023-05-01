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
	UltrasonicSensor(const int16_t id, const int pulseMaxTimeoutMicroSeconds) :
			UltrasonicSensor(id, -1, -1, pulseMaxTimeoutMicroSeconds) {
		// nothing to do here...
	}

	UltrasonicSensor(const int16_t id, const int txPin, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
			k_id(id), k_txPin(-1), k_rxPin(rxPin), k_pulseMaxTimeoutMicroSeconds(pulseMaxTimeoutMicroSeconds),
			k_maxDistance((k_pulseMaxTimeoutMicroSeconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2.0f) {
		SerialLogger::debug(F("Initiating ultrasonic sensor %s on rxPin=%d with id=%d with txPin=%d and max "
		                      "possible distance at %f"), SpiCommands::getNameFromId(id), k_rxPin, k_id, k_txPin,
		                    k_maxDistance);
		if (k_txPin > 0) {
			pinMode(k_txPin, OUTPUT);
			digitalWrite(k_txPin, LOW);
		}
		if (k_rxPin > 0) {
			pinMode(k_rxPin, INPUT);
		}
		_latestDistance = 0;
	}

	// having const values is more valuable than this copy-assignment; if you need to move use (smart) pointers
	UltrasonicSensor &UltrasonicSensor::operator=(const UltrasonicSensor &ultrasonicSensor) = delete;

	~UltrasonicSensor() {
		if (k_txPin > 0) {
			digitalWrite(k_txPin, LOW);
		}
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

	void updateLatestDistanceWithTx();

protected:
	void triggerTx() const {
		if (k_txPin > 0) {
			digitalWrite(k_txPin, LOW);
			delayMicroseconds(2);
			digitalWrite(k_txPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(k_txPin, LOW);
		}
	};

	virtual void updateLatestDistanceWithoutTx();

	float weightNewDistance(const float new_distance) {
		const char *sensorName = SpiCommands::getNameFromId(k_id);
		if (_latestDistance < NO_ECHO_DISTANCE && new_distance >= k_maxDistance) {
			// The object got closer to the robot and is now probably "at" the robot leading to no echo due to the object. Setting to zero!
			_latestDistance = 0.0f;
			SerialLogger::debug(F("Setting distance of sensor=%s with id %d at rx pin %d to zero"),
			                    sensorName, k_id, k_rxPin);
		} else {
			_latestDistance = (_latestDistance + new_distance) / 2.0f;
			SerialLogger::info(F("Setting distance of sensor=%s with id %d at rx pin %d to %f/%f"),
			                   sensorName, k_id, k_rxPin, _latestDistance, new_distance);
		}

		return _latestDistance;
	}

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
	                                           Timer<> &timer);

	static void schedule(Timer<> &timer, UltrasonicSensors *ultrasonicSensors);

	UltrasonicSensors(const int txPin, const int *rxPins, const int16_t ids[], const int amountSensors,
	                  const int pulseMaxTimeoutMicroSeconds);

	UltrasonicSensors(const int amountSensors);

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


protected:
	const int k_amountSensors;
	UltrasonicSensor **_ultrasonicSensors;
	int _registeredSensors = 0;

private:
	volatile int _nextSensorIndex = 0;
};

#endif // ULTRASONIC_SENSORS_H
