#include "ultrasonic_sensors.h"

void UltrasonicSensor::triggerTx(const int txPin) {
	digitalWrite(txPin, LOW);
	delayMicroseconds(2);
	digitalWrite(txPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(txPin, LOW);
}

UltrasonicSensors *UltrasonicSensors::getFromScheduled(const int txPin, const int rxPins[], const int16_t ids[],
													   const int amountSensors, const int pulseMaxTimeoutMicroSeconds,
													   Timer<> &timer, const int sensoring_frequency_delay) {
	UltrasonicSensors *ultrasonicSensors = new UltrasonicSensors(txPin, rxPins, ids, amountSensors,
																 pulseMaxTimeoutMicroSeconds);
	timer.every(sensoring_frequency_delay, [](UltrasonicSensors *ultrasonicSensors) -> bool {
		if (ultrasonicSensors == nullptr) {
			SerialLogger::error(F("UltrasonicSensor is nullptr. Something wrong, stopping timer iteration"));
			return false;
		} else {
			ultrasonicSensors->updateNextDistanceFromSensors();
			return true; // to repeat the action - false to stop
		}
	}, ultrasonicSensors);
	return ultrasonicSensors;
}

UltrasonicSensors::UltrasonicSensors(const int txPin, const int rxPins[], const int16_t ids[], const int amountSensors,
									 const int pulseMaxTimeoutMicroSeconds) :
		k_txPin(txPin), k_amountSensors(amountSensors) {

	_ultrasonicSensors = (UltrasonicSensor **) malloc(k_amountSensors * sizeof _ultrasonicSensors);
	int16_t ids_check[k_amountSensors] = {-1};
	int rxPins_check[k_amountSensors] = {-1};

	for (int i = 0; i < k_amountSensors; i++) {
		const int rxPin = rxPins[i];
		const int16_t id = ids[i];
		SerialLogger::info(F("Creating sensor %d/%d with rxPin=%d and id=%d"), i + 1, k_amountSensors, rxPin, id);

		bool duplicate = false;
		for (int j = 0; j < k_amountSensors; j++) {
			if (ids_check[j] == id || rxPins_check[j] == rxPin) {
				SerialLogger::error(F("Attempt to add the same sensor twice with id %d on rx pin %d where at index %d"
									  " of the check arrays the same value already exists with id=%d and rxpin=%d"), id,
									rxPin, j, ids_check[j], rxPins_check[j]);
				duplicate = true;
				break;
			}
		}
		if (!duplicate) {
			ids_check[i] = id;
			rxPins_check[i] = rxPin;
			if (rxPin <= MAX_ARDUINO_PINS && rxPin >= 2) {
				_ultrasonicSensors[_registeredSensors++] = new UltrasonicSensor(id, k_txPin, rxPin,
																				pulseMaxTimeoutMicroSeconds);
			} else {
				SerialLogger::warn(F("Cannot add sensor %d/%d at rx pin %d which is out of range. Max Pin is %d. "
									 "Assuming a bad malfunctioning and stopping..."), i + 1, k_amountSensors,
								   rxPin, MAX_ARDUINO_PINS);
				break;
			}
		}
	}
	if (_registeredSensors == k_amountSensors) {
		SerialLogger::info(F("Scheduling ultrasonic sonic distance sensoring from pin %d to echo pins"), k_txPin);
	} else {
		SerialLogger::error(F("Could only add %d/%d sensors. Expect issues."), _registeredSensors, k_amountSensors);
	}
}

UltrasonicSensors::~UltrasonicSensors() {
	digitalWrite(k_txPin, LOW);
	for (int i = 0; i < _registeredSensors; i++) {
		delete _ultrasonicSensors[i];
	}
	delete _ultrasonicSensors;
}

void UltrasonicSensors::updateNextDistanceFromSensors() {
	/* Note: updateDistanceFromSensors performs a round-robin. This has the drawback of imposing a delay of up to
	  n x pulseMaxTimeoutMicroSeconds at worst where n is the amount of sensors to update distance from. Given the
	  fact that we might operate in a timer callback function, we cannot delay this execution or other callbacks
	  might get broken by it. Hence, we still perform a round robin but release the timer after one sensors was caputured
	  to allow other callbacks to get executed without (or smaller) delay.
	*/
	if (_registeredSensors > 0) {
		UltrasonicSensor *sensor = _ultrasonicSensors[_nextSensorIndex];
		sensor->updateLatestDistanceWithTx();
		_nextSensorIndex = (_nextSensorIndex + 1) % _registeredSensors;
	}
}

void UltrasonicSensors::updateDistanceFromSensors() {
	/* Note: We need to make a round-robin or the distance of one sensor will be incorrect if one update call takes too long
	  (e. g. if pulseMaxTimeoutMicroSeconds was reached). This has the drawback of imposing a delay of up to
	  n x pulseMaxTimeoutMicroSeconds at worst where n is the amount of sensors to update distance from.
	*/
	for (int i = 0; i < _registeredSensors; i++) {
		UltrasonicSensor *sensor = _ultrasonicSensors[i];
		sensor->updateLatestDistanceWithTx();
	}
}

float UltrasonicSensors::getLatestDistanceFromSensorByPin(const int sensorPin) const {
	float distance = -1.0;
	for (int i = 0; i < _registeredSensors; i++) {
		const UltrasonicSensor *sensor = _ultrasonicSensors[i];
		if (sensorPin == sensor->getRxPin()) {
			distance = sensor->getLatestDistance();
			break;
		}
	}
	if (distance < 0) {
		SerialLogger::warn(F("Could not find ultrasonic sensor on rx pin %d"), sensorPin);
	}
	return distance;
}

float UltrasonicSensors::getLatestDistanceFromSensorById(const int16_t id) const {
	float distance = -1.0;
	for (int i = 0; i < _registeredSensors; i++) {
		const UltrasonicSensor *sensor = _ultrasonicSensors[i];
		if (id == sensor->getId()) {
			distance = sensor->getLatestDistance();
			break;
		}
	}
	if (distance < 0) {
		SerialLogger::warn(F("Could not find ultrasonic sensor from id %d"), id);
	}
	return distance;
}
