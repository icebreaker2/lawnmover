#include "esp32_spi_master.h"

#include <Arduino.h>
#include <serial_logger.h>

int _delayPerSpiRead;
// TODO Fix the timer lib to handle capture arguments....
int _slavePin1 = -1;
SPIClass *_spiClass1 = nullptr;
SPISettings _spiSettings1;
void (*_slaveCallback1)(const int, const int, SPIClass*) = nullptr;
int _slavePin2 = -1;
SPIClass *_spiClass2 = nullptr;
SPISettings _spiSettings2;
void (*_slaveCallback2)(const int, const int, SPIClass*) = nullptr;
int _slavePin3 = -1;
SPIClass *_spiClass3 = nullptr;
SPISettings _spiSettings3;
void (*_slaveCallback3)(const int, const int, SPIClass*) = nullptr;

Esp32SpiMaster::Esp32SpiMaster(const int clockPin, const int misoPin, const int mosiPin, const long frequency, const int delayPerSpiRead) :
    k_clockPin(clockPin), k_misoPin(misoPin), k_mosiPin(mosiPin), k_frequency(frequency), k_delayPerSpiRead(delayPerSpiRead) {
    // Quick & dirty handle spi delay for timers C-like functional pointer (post to outer scope)
    _delayPerSpiRead = k_delayPerSpiRead;

}

Esp32SpiMaster::~Esp32SpiMaster() {
    //for (auto spiClass : m_spiSlaves) {
    //    delete spiClass;
    //}
}

void Esp32SpiMaster::addSlave(const int slavePin, const int readStateDelay, const long clockDivider,
                              Timer<> &timer, void (*slaveCallback)(const int, const int, SPIClass *spiClass)) {
    // See: https://forum.arduino.cc/t/esp32-spi-comunication-on-sd-module/598345
    /*
        The default pins for SPI on the ESP32.
        HSPI
        MOSI = GPIO13
        MISO = GPIO12
        CLK = GPIO14
        CD = GPIO15

        VSPI
        MOSI = GPIO23
        MISO = GPIO19
        CLK/SCK = GPIO18
        CS/SS = GPIO5
    */

    SPIClass *spiClass = new SPIClass(VSPI);
    // Put SCK, MOSI, SS pins into output mode also put SCK, MOSI into LOW state, and SS into HIGH state.
    // Then put SPI hardware into Master mode and turn SPI on Begins the SPI communication
    spiClass->begin(k_clockPin, k_misoPin, k_mosiPin, slavePin);
    //Sets clock for SPI communication at 8 (16/8=2Mhz)
    // TODO: No need to set it if we already set frequency?!?!
    //spiClass->setClockDivider(clockDivider);

    // Setting SlaveSelect as HIGH (So master doesnt connect with slave)
    digitalWrite(slavePin, HIGH);

    // TODO fix timer lib to use captures
    if (_slaveCallback1 == nullptr) {
        _slavePin1 = slavePin;
        _slaveCallback1 = slaveCallback;
        _spiClass1 = spiClass;
        _spiSettings1 = SPISettings(k_frequency, MSBFIRST, SPI_MODE0);
        timer.every(readStateDelay, [](void*) -> bool {
            //use it as you would the regular arduino SPI API
            _spiClass1->beginTransaction(_spiSettings1);
            digitalWrite(_slavePin1, LOW); //pull SS slow to prep other end for transfer
            (*_slaveCallback1)(_slavePin1, _delayPerSpiRead, _spiClass1);
            digitalWrite(_slavePin1, HIGH); //pull ss high to signify end of data transfer
            _spiClass1->endTransaction();
            return true; // to repeat the action - false to stop
        });
    } else if (_slaveCallback2 == nullptr) {
        _slavePin2 = slavePin;
        _slaveCallback2 = slaveCallback;
        _spiClass2 = spiClass;
        _spiSettings2 = SPISettings(k_frequency, MSBFIRST, SPI_MODE0);
        timer.every(readStateDelay, [](void*) -> bool {
            //use it as you would the regular arduino SPI API
            _spiClass2->beginTransaction(_spiSettings2);
            digitalWrite(_slavePin2, LOW); //pull SS slow to prep other end for transfer
            (*_slaveCallback2)(_slavePin2, _delayPerSpiRead, _spiClass2);
            digitalWrite(_slavePin2, HIGH); //pull ss high to signify end of data transfer
            _spiClass2->endTransaction();
            return true; // to repeat the action - false to stop
        });
    } else if (_slaveCallback3 == nullptr) {
        _slavePin3 = slavePin;
        _slaveCallback3 = slaveCallback;
        _spiClass3 = spiClass;
        _spiSettings3 = SPISettings(k_frequency, MSBFIRST, SPI_MODE0);
        timer.every(readStateDelay, [](void*) -> bool {
            //use it as you would the regular arduino SPI API
            _spiClass3->beginTransaction(_spiSettings3);
            digitalWrite(_slavePin3, LOW); //pull SS slow to prep other end for transfer
            (*_slaveCallback3)(_slavePin3, _delayPerSpiRead, _spiClass3);
            digitalWrite(_slavePin3, HIGH); //pull ss high to signify end of data transfer
            _spiClass3->endTransaction();
            return true; // to repeat the action - false to stop
        });
    } else {
        SerialLogger::error("Cannot add new Slave to SPI on pin: %d", slavePin);
    }

    //m_spiSlaves.push_back(spiClass);
}
