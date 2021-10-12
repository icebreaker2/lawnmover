#include "SpiSlave.h"
#include <Arduino.h>
#include <SPI.h>

#define SCK_PIN   13  // D13 = pin19 = PortB.5
#define MISO_PIN  12  // D12 = pin18 = PortB.4
#define MOSI_PIN  11  // D11 = pin17 = PortB.3
#define SS_PIN    10  // D10 = pin16 = PortB.2

#define UL unsigned long
#define US unsigned short


volatile char buf [20] = "Hello, world!";
volatile int pos;
volatile bool active;

void arduino_spi_slave() {
    // Set MISO output, all others input
    pinMode(SCK_PIN, INPUT);
    pinMode(MOSI_PIN, INPUT);
    pinMode(MISO_PIN, OUTPUT);  // (only if bidirectional mode needed)
    pinMode(SS_PIN, INPUT);

    /*  Setup SPI control register SPCR
        SPIE - Enables the SPI interrupt when 1
        SPE - Enables the SPI when 1
        DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
        MSTR - Sets the Arduino in master mode when 1, slave mode when 0
        CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
        CPHA - Samples data on the trailing edge of the data clock when 1, leading edge when 0
        SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)   */

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
    byte c = SPDR;

    if (c == 1) {
        // starting new sequence?
        active = true;
        pos = 0;
        SPDR = buf [pos++];   // send first byte
        Serial.printf("Resetting and sending first character %s\n", buf [pos]);
        return;
    }

    if (!active) {
        Serial.printf("Slave not yet activated\n");
        SPDR = 0;
        return;
    }

    SPDR = buf [pos];
    if (buf [pos] == 0 || ++pos >= sizeof (buf)) {
        Serial.printf("End of transmitted String reached\n");
        active = false;
    }
}  // end of interrupt service routine (ISR) SPI_STC_vect

SpiSlave::SpiSlave() {
    arduino_spi_slave();
}

SpiSlave::~SpiSlave() {
    //SPI.end();
}
