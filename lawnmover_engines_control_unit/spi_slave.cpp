#include "spi_slave.h"
#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>

volatile char buf [20] = "Hello world!lalalal";
volatile int pos;
volatile bool active;

SpiSlave::SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin) {
    AddArduinoSpiSlave(sck_pin, miso_pin, mosi_pin, ss_pin);
}

SpiSlave::~SpiSlave() {
    //SPI.end();
}

void SpiSlave::AddArduinoSpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin) {
    pinMode(sck_pin, INPUT);
    pinMode(mosi_pin, INPUT);
    pinMode(miso_pin, OUTPUT);  // (only if bidirectional mode needed)
    pinMode(ss_pin, INPUT);

    /*  Setup SPI control register SPCR
        SPIE - Enables the SPI interrupt when 1
        SPE - Enables the SPI when 1
        DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
        MSTR - Sets the Arduino in master mode when 1, slave mode when 0
        CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
        CPHA - Samples data on the trailing edge of the data clock when 1, leading edge when 0
        SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)
    */

    // enable SPI subsystem and set correct SPI mode
    // SPCR = (1<<SPE)|(0<<DORD)|(0<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(1<<SPR0);

    // SPI status register: SPSR
    // SPI data register: SPDR
    // turn on SPI in slave mode
    SPCR |= bit(SPE);
    // turn on interrupts
    SPCR |= bit(SPIE);
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
    const byte c = SPDR;
    SerialLogger::debug("Received: %d", c);
    if (c == 1) {
        // starting new sequence?
        active = true;
        pos = 0;
        SerialLogger::debug("Resetting");
    } else if (c == 0) {
        SerialLogger::warn("Received 0. Omitting to prevent bad command interpretation. Returning 0");
        SPDR = 0;
        return;
    } else {
        if (!active) {
            SerialLogger::warn("Slave not yet activated");
            SPDR = 0;
            return;
        }
        if (buf [pos] == '\0') {
            SerialLogger::info("End of transmitted String reached");
            active = false;
        }
    }
    SerialLogger::debug("Sending back: %c", buf[pos]);
    SPDR = buf[pos];
    pos++;
}  // end of interrupt service routine (ISR) SPI_STC_vect
