#include "ultrasonic_sensors.h"

void UltrasonicSensor::triggerTx(const int txPin) {
  digitalWrite(txPin, LOW);
  delayMicroseconds(2);
  digitalWrite(txPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(txPin, LOW);
}

UltrasonicSensors *UltrasonicSensors::getFromScheduled(const int txPin, const int rxPins [], const int amountSensors, const int pulseMaxTimeoutMicroSeconds, Timer<> &timer, const int sensoring_frequency_delay) {
  UltrasonicSensors *ultrasonicSensors = new UltrasonicSensors(txPin, rxPins, amountSensors, pulseMaxTimeoutMicroSeconds);
  timer.every(sensoring_frequency_delay, [](UltrasonicSensors * ultrasonicSensors) -> bool {
    ultrasonicSensors->updateDistanceFromSensors();
    return true; // to repeat the action - false to stop
  }, ultrasonicSensors);
  return ultrasonicSensors;
}

UltrasonicSensors::UltrasonicSensors(const int txPin, const int rxPins[], const int amountSensors, const int pulseMaxTimeoutMicroSeconds)  :
  k_txPin(txPin), k_amountSensors(amountSensors) {

  _ultrasonicSensors = (UltrasonicSensor **) malloc(k_amountSensors * sizeof * _ultrasonicSensors);
  _nextSensorIndex = 0;
  
  char echoPinsString[MAX_ARDUINO_PINS * 4 + 5];
  strcat(echoPinsString, "pins ");
  for (int i = 0; i < k_amountSensors; i++) {
    const int rxPin = rxPins[i];
    if (rxPin <= MAX_ARDUINO_PINS) {
      _ultrasonicSensors[i] = new UltrasonicSensor(k_txPin, rxPin, pulseMaxTimeoutMicroSeconds);
      char buf[4];
      sprintf(buf, "%d, ", rxPin);
      strcat(echoPinsString, buf);
    } else {
      SerialLogger::warn("Cannot add sensor at rx pin %d which is out of range. Max Pin is %d", rxPin, MAX_ARDUINO_PINS);
    }
  }

  SerialLogger::info("Scheduling ultrasonic sonic distance sesnoring from pin %d to echo on %s", k_txPin, echoPinsString);
}

UltrasonicSensors::~UltrasonicSensors() {
  digitalWrite(k_txPin, LOW);
  for (int i = 0; i < k_amountSensors; i++) {
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
  UltrasonicSensor *sensor = _ultrasonicSensors[_nextSensorIndex];
  sensor->updateLatestDistanceWithTx();
  _nextSensorIndex = (_nextSensorIndex + 1) % k_amountSensors;
}

void UltrasonicSensors::updateDistanceFromSensors() {
  /* Note: We need to make a round-robin or the distance of one sensor will be incorrect if one update call takes too long
    (e. g. if pulseMaxTimeoutMicroSeconds was reached). This has the drawback of imposing a delay of up to
    n x pulseMaxTimeoutMicroSeconds at worst where n is the amount of sensors to update distance from.
  */
  for (int i = 0; i < k_amountSensors; i++) {
    UltrasonicSensor *sensor = _ultrasonicSensors[i];
    sensor->updateLatestDistanceWithTx();
  }
}

float UltrasonicSensors::getLatestDistanceFromSensor(const int sensorPin) const {
  float distance = -1.0;
  for (int i = 0; i < k_amountSensors; i++) {
    const UltrasonicSensor *sensor = _ultrasonicSensors[i];
    if (sensorPin == sensor->getRxPin()) {
      distance = sensor->getLatestDistance();
      break;
    }
  }
  if (distance < 0) {
    SerialLogger::warn("Could not find ultrasonic sensor on rx pin %d", sensorPin);
  }
  return distance;
};
