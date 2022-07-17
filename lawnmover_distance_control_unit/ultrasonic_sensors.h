#ifndef ULTRASONIC_SENSORS_H
#define ULTRASONIC_SENSORS_H

#include <Arduino.h>
#include <arduino_timer_uno.h>
#include <serial_logger.h>

#define SENSORING_FREQUENCY_DELAY 30
#define MAX_ARDUINO_PINS 13
#define ULTRASONIC_CM_PER_MICROSECOND_AIR 29
#define NO_ECHO_DISTANCE 3.0f

class UltrasonicSensor {
  public:
    UltrasonicSensor(const int txPin, const int rxPin, const int pulseMaxTimeoutMicroSeconds) :
      k_txPin(txPin), k_rxPin(rxPin), k_pulseMaxTimeoutMicroSeconds(pulseMaxTimeoutMicroSeconds),
      k_maxDistance((k_pulseMaxTimeoutMicroSeconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2) {
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
      long distance_cm = (duration_microseconds / ULTRASONIC_CM_PER_MICROSECOND_AIR) / 2 ;
      const float new_latestDistance = (_latestDistance + distance_cm) / 2;
      if (_latestDistance < NO_ECHO_DISTANCE && new_latestDistance >= k_maxDistance) {
        // The object got closer to the robot and is now probably "at" the robot leading to no echo due to the object. Setting to zero!
        _latestDistance = 0.0f;
        SerialLogger::debug("Setting distance of sensor at pin %d to zero", k_rxPin);
      } else {
        _latestDistance = new_latestDistance;
      }
    };

  private:
    const int k_txPin;
    const int k_rxPin;
    const int k_pulseMaxTimeoutMicroSeconds;
    const int k_maxDistance;
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


    void updateNextDistanceFromSensors();

    void updateDistanceFromSensors();

    float getLatestDistanceFromSensor(const int sensorPin) const;

  private:
    const int k_txPin;
    const int k_amountSensors;

    UltrasonicSensor **_ultrasonicSensors;
    volatile int _nextSensorIndex;
};

#endif // ULTRASONIC_SENSORS_H
