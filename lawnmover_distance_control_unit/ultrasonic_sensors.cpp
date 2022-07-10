#include "ultrasonic_sensors.h"
#include <serial_logger.h>

int _txPin = 0;
int *_rxPins;
int _rxPinsLength = 0;

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


void UltrasonicSensors::updateDistanceFromSensors() {
  UltrasonicSensor::triggerTx(k_txPin);
  for (int i = 0; i < k_amountSensors; i++) {
    // TODO: make round-robin slows it down but doing it this way may case wrong distances if one update call takes to long (e. g. if maxTimeout is too large)
    UltrasonicSensor *sensor = _ultrasonicSensors[i];
    sensor->updateLatestDistanceWithoutTx();
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
