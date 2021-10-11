#include "SpiMaster.h"

#include <Arduino.h>
#include <SerialLogger.h>

int _delayPerSpiRead;
// TODO Fix the timer lib to handle capture arguments....
int _slavePin1 = -1;
void (*_slaveCallback1)(const int, const int) = nullptr;
int _slavePin2 = -1;
void (*_slaveCallback2)(const int, const int) = nullptr;
int _slavePin3 = -1;
void (*_slaveCallback3)(const int, const int) = nullptr;

SpiMaster::SpiMaster(const long clockDivider, const int delayPerSpiRead) :
    k_delayPerSpiRead(delayPerSpiRead) {
    // Quick & dirty handle spi delay for timers C-like functional pointer (post to outer scope)
    _delayPerSpiRead = k_delayPerSpiRead;

    // Put SCK, MOSI, SS pins into output mode
    // also put SCK, MOSI into LOW state, and SS into HIGH state.
    // Then put SPI hardware into Master mode and turn SPI on
    //Begins the SPI commnuication
    SPI.begin();
    //Sets clock for SPI communication at 8 (16/8=2Mhz)
    SPI.setClockDivider(clockDivider);
}

SpiMaster::~SpiMaster() {
    SPI.end();
}

void SpiMaster::addSlave(const int slavePin, const int readStateDelay, Timer<> &timer, void (*slaveCallback)(const int, const int)) {
    // Setting SlaveSelect as HIGH (So master doesnt connnect with slave)
    digitalWrite(slavePin, HIGH);

    // TODO fix timer lib to use caputures
    if (_slaveCallback1 == nullptr) {
        _slavePin1 = slavePin;
        _slaveCallback1 = slaveCallback;
        timer.every(readStateDelay, [](void*) -> bool {
            (*_slaveCallback1)(_slavePin1, _delayPerSpiRead);
            return true; // to repeat the action - false to stop
        });
    } else if (_slaveCallback2 == nullptr) {
        _slavePin2 = slavePin;
        _slaveCallback2 = slaveCallback;
        timer.every(readStateDelay, [](void*) -> bool {
            (*_slaveCallback2)(_slavePin2, _delayPerSpiRead);
            return true; // to repeat the action - false to stop
        });
    } else if (_slaveCallback3 == nullptr) {
        _slavePin3 = slavePin;
        _slaveCallback3 = slaveCallback;
        timer.every(readStateDelay, [](void*) -> bool {
            (*_slaveCallback3)(_slavePin3, _delayPerSpiRead);
            return true; // to repeat the action - false to stop
        });
    } else {
        SerialLogger::error("Cannot add new Slave to SPI on pin: %d", slavePin);
    }
}
