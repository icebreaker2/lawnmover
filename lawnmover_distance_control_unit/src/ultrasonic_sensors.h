#ifndef ULTRASONIC_SENSORS_H
#define ULTRASONIC_SENSORS_H

#include <Arduino.h>
#include <arduino_timer_uno.h>
#include <serial_logger.h>
#include <spi_commands.h>

#define SENSORING_FREQUENCY_DELAY 45
#define MAX_ARDUINO_PINS 13
#define ULTRASONIC_CM_PER_MICROSECOND_AIR 29

class UltrasonicSensor {
public:
	UltrasonicSensor(const int16_t id, const int pulseMaxTimeoutMicroSeconds) :
			UltrasonicSensor(id, -1, -1, pulseMaxTimeoutMicroSeconds) {
		// nothing to do here...
	};

    UltrasonicSensor(const int16_t id, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
			UltrasonicSensor(id, -1, rxPin, pulseMaxTimeoutMicroSeconds) {
		// nothing to do here...
	};

	UltrasonicSensor(const int16_t id, const int txPin, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
			k_id(id), k_txPin(txPin), k_rxPin(rxPin), k_pulseMaxTimeoutMicroSeconds(pulseMaxTimeoutMicroSeconds),
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
		_latestDistance = 0.0f;
	};

	// having const values is more valuable than this copy-assignment; if you need to move use (smart) pointers
	UltrasonicSensor &UltrasonicSensor::operator=(const UltrasonicSensor &ultrasonicSensor) = delete;

	~UltrasonicSensor() {
		if (k_txPin > 0) {
			digitalWrite(k_txPin, LOW);
		}
	};

	int getRxPin() const {
		return k_rxPin;
	};

	int16_t getId() const {
		return k_id;
	};

	float getLatestDistance() const {
		return _latestDistance;
	};

    void updateLatestDistanceWithTx() {
	    triggerTx();
	    updateLatestDistanceWithoutTx();
    };

protected:
	void triggerTx() const {
		if (k_txPin > 0) {
			digitalWrite(k_txPin, LOW);
			delayMicroseconds(2);
			digitalWrite(k_txPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(k_txPin, LOW);
		} else {
	        SerialLogger::warn(F("No txPin set. Cannot send ultrasonic wave for UltrasonicSensor %d."), k_id);
	    }
	};

    void updateLatestDistanceWithoutTx() {
        if (k_txPin > 0) {
            // Use pulseInLong to work with SPI interrupts and timer execution?!
            long duration_microseconds = pulseInLong(k_rxPin, HIGH, k_pulseMaxTimeoutMicroSeconds);
            if (duration_microseconds == 0) {
                // no echo read before timeout
                duration_microseconds = k_pulseMaxTimeoutMicroSeconds;
            }
            // The signal went back and forth but we do only need one distance
            const float new_distance = (duration_microseconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2.0f;
            SerialLogger::trace(F("UltrasonicSensor %d has distance update: new %f vs. old %f"),
                                  k_id, new_distance, _latestDistance);
            weightNewDistance(new_distance);
        } else {
            SerialLogger::warn(F("No txPin set. Cannot measure latest distance from ultrasonic response "
                                 "for UltrasonicSensor %d."), k_id);
        }
    };

	float weightNewDistance(const float new_distance) {
		const char *sensorName = SpiCommands::getNameFromId(k_id);
        _latestDistance = (_latestDistance + new_distance) / 2.0f;
        SerialLogger::trace(F("Setting distance of sensor=%s with id %d at rx pin %d to %f/%f"),
			                   sensorName, k_id, k_rxPin, _latestDistance, new_distance);
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
	                                           Timer<> &timer, const int updateFrequencyMS = SENSORING_FREQUENCY_DELAY);

	static void schedule(Timer<> &timer, UltrasonicSensors *ultrasonicSensors, const int updateFrequencyMS);

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
			    // TODO this depends on the configuration....we should fix this to work dynamically
				SerialLogger::debug(F("F: %f, FR: %f, FL: %f, L: %f, R: %f, BR: %f, BL: %f"),
				                   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_COMMAND),
				                   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_RIGHT_COMMAND),
				                   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_FRONT_LEFT_COMMAND),
                                   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_LEFT_COMMAND),
				                   ultrasonicSensors->getLatestDistanceFromSensorById(OBSTACLE_RIGHT_COMMAND),
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
