#include "spi_slave.h"
#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>

volatile char buf [8] = {0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01};
volatile int pos = 0;
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
uint8_t id_bytes [2] = {0};
uint8_t value_bytes [4] = {0};
ISR (SPI_STC_vect) {
    const byte c = SPDR;
    SerialLogger::debug("Received: %d", c);
    if (pos == 0 || pos == 1) {
        id_bytes[pos] = c;
        SPDR = c;
    } else if (pos < 6) {
        value_bytes[pos] = c;
        SPDR = c;
    } else {
        if (c != 0xFF) {
            SerialLogger::warn("Received bad ack id request id byte %c", c);
        }
        SPDR = id_bytes[pos % 2];
    }

    if (pos == 7) {
        pos = 0;
        Serial.print("Received: ");
        Serial.print(id_bytes[0], HEX);
        Serial.print(id_bytes[1], HEX);
        Serial.print(value_bytes[0], HEX);
        Serial.print(value_bytes[1], HEX);
        Serial.print(value_bytes[2], HEX);
        Serial.print(value_bytes[3], HEX);
        Serial.print(id_bytes[0], HEX);
        Serial.print(id_bytes[1], HEX);
        Serial.println("");
        SerialLogger::debug("Resetting");
    } else {
        pos++;
    }

}  // end of interrupt service routine (ISR) SPI_STC_vect
