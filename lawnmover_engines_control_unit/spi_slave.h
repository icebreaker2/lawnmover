class SpiSlave {
    public:
        SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin);
        ~SpiSlave();

    private:
        void AddArduinoSpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin);
};
