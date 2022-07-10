#ifndef ULTRASONIC_SENSORS_H
#define ULTRASONIC_SENSORS_H

#include <Arduino.h>
#include <arduino_timer_uno.h>

#define SENSORING_FREQUENCY_DELAY 80
#define MAX_ARDUINO_PINS 13
#define ULTRASONIC_CM_PER_MICROSECOND_AIR 29


class UltrasonicSensor {
  public:
    UltrasonicSensor(const int txPin, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
      k_txPin(txPin), k_rxPin(rxPin), k_pulseMaxTimeoutMicroSeconds(pulseMaxTimeoutMicroSeconds) {
      pinMode(k_txPin, OUTPUT);
      digitalWrite(k_txPin, LOW);
      pinMode(k_rxPin, INPUT);
      _latestDistance = 0.0;
    }

    // having const values is more valuable than this copy-assignment; if you need to move use (smart) pointers
    UltrasonicSensor& UltrasonicSensor::operator=(const UltrasonicSensor &ultrasonicSensor) = delete;

    ~UltrasonicSensor() {
      digitalWrite(k_txPin, LOW);
    }

    int getRxPin() const {
      return k_rxPin;
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
      long distance_cm = duration_microseconds / ULTRASONIC_CM_PER_MICROSECOND_AIR / 2 ;
      // TODO compare to last value; drop spikes if bad. Use mean for now which should be fine in a high frequency updates matter
      _latestDistance = (_latestDistance + distance_cm) / 2;
    };

  private:
    const int k_txPin;
    const int k_rxPin;
    const int k_pulseMaxTimeoutMicroSeconds;
    // Note: Assuming there is one writer and multiple readers, i. e. volatile is enough
    volatile float _latestDistance;
};

class UltrasonicSensors {
  public:

    static UltrasonicSensors *getFromScheduled(const int txPin, const int rxPins [], const int amountSensors, const int pulseMaxTimeoutMicroSeconds, Timer<> &timer, const int sensoring_frequency_delay = SENSORING_FREQUENCY_DELAY);

    UltrasonicSensors(const int txPin, const int *rxPins, const int amountSensors, const int pulseMaxTimeoutMicroSeconds);

    ~UltrasonicSensors();

    int getAmountSensors() const {
      return k_amountSensors;
    };

    void updateDistanceFromSensors();

    float getLatestDistanceFromSensor(const int sensorPin) const;

  private:
    const int k_txPin;
    const int k_amountSensors;

    UltrasonicSensor **_ultrasonicSensors;
};

#endif // ULTRASONIC_SENSORS_H
