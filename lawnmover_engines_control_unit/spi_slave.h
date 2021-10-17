#include <arduino_timer_uno.h>

class SpiSlave {
    public:
        SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
                 bool (*leftWheelSteeringCommand)(int16_t), bool (*rightWheelSteeringCommand)(int16_t),
                 bool (*motorSpeedCommand)(int16_t));
        ~SpiSlave() {
            // nothing to do...
        };

        void addSlavePrinting(Timer<> &timer, const int interval);

    private:
};
