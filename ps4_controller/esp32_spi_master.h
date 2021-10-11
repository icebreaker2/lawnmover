#include <arduino_timer_uno.h>
#include <esp32-hal-spi.h>
// See https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
#include <SPI.h>

class Esp32SpiMaster {
    public:
        Esp32SpiMaster(const int clockPin, const int misoPin, const int mosiPin, const long frequency = 2000000,
                       const int delayPerSpiRead = 15);
        ~Esp32SpiMaster();

        void addSlave(const int slavePin, const int readStateDelay, const long clockDivider,
                      Timer<> &timer, void (*slaveCallback)(const int, const int, SPIClass *spiClass));

    private:
        const int k_clockPin;
        const int k_misoPin;
        const int k_mosiPin;
        const int k_frequency;
        const int k_delayPerSpiRead;
};
