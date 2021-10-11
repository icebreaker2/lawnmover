#include <arduino_timer_uno.h>
#include <SPI.h>

class SpiMaster {
    public:
        SpiMaster(const long clockDivider = SPI_CLOCK_DIV8, const int delayPerSpiRead = 15);
        ~SpiMaster();

        void addSlave(const int slavePin, const int readStateDelay, Timer<> &timer, void (*slaveCallback)(const int, const int));

    private:
        const int k_delayPerSpiRead;
};
