#include "spi_slave.h"
#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>

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
volatile int pos = 0;
uint8_t id_bytes [2];
uint8_t value_bytes [4];
uint8_t rx_buffer [9];
uint8_t tx_buffer [9];
ISR (SPI_STC_vect) {
    // TODO Printing consumes too much time. Slave does not respond in time.
    const uint8_t c = SPDR;
    //SerialLogger::debug("Received: %d", c);
    rx_buffer[pos] = c;
    if (pos == 0 || pos == 1) {
        id_bytes[pos] = c;
        //Serial.println(pos);
        //Serial.print("Response: ");
        //Serial.println(c, HEX);

        SPDR = c;
        tx_buffer[pos] = c;
    } else if (pos < 6) {
        value_bytes[pos - 2] = c;
        //Serial.println(pos);
        //Serial.print("Response: ");
        //Serial.println(c, HEX);
        SPDR = c;
        tx_buffer[pos] = c;
    } else if (pos < 8) {
        if (c != 0xFF) {
            SerialLogger::warn("Received bad ack id request id byte %c", c);
        }
        SPDR = id_bytes[pos % 2];
        tx_buffer[pos] = id_bytes[pos % 2];
        //Serial.println(pos);
        //Serial.print("Response: ");
        //Serial.println(id_bytes[pos % 2], HEX);
    } else {
        //Serial.println(pos);
        // For the master to receive the nth byte we need to send a n+1 byte
        SPDR = 0;
        //Serial.println("Initiating new communication");
    }

    if (pos == 8) {
        pos = 0;
        /*  Serial.print("Received: ");
            Serial.print(rx_buffer[0], HEX);
            Serial.print(rx_buffer[1], HEX);
            Serial.print(rx_buffer[2], HEX);
            Serial.print(rx_buffer[3], HEX);
            Serial.print(rx_buffer[4], HEX);
            Serial.print(rx_buffer[5], HEX);
            Serial.print(rx_buffer[6], HEX);
            Serial.print(rx_buffer[7], HEX);
            Serial.print(rx_buffer[8], HEX);
            Serial.println("");
            Serial.print("Send: ");
            Serial.print(tx_buffer[0], HEX);
            Serial.print(tx_buffer[1], HEX);
            Serial.print(tx_buffer[2], HEX);
            Serial.print(tx_buffer[3], HEX);
            Serial.print(tx_buffer[4], HEX);
            Serial.print(tx_buffer[5], HEX);
            Serial.print(tx_buffer[6], HEX);
            Serial.print(tx_buffer[7], HEX);
            Serial.print(tx_buffer[8], HEX);
            Serial.println("");
            SerialLogger::debug("Resetting");*/
    } else {
        pos++;
    }

}  // end of interrupt service routine (ISR) SPI_STC_vect

void SpiSlave::addSlavePrinting(Timer<> &timer, const int interval) {
    timer.every(interval, [](void*) -> bool {
        Serial.print("Received: ");
        Serial.print(rx_buffer[0], HEX);
        Serial.print(rx_buffer[1], HEX);
        Serial.print(rx_buffer[2], HEX);
        Serial.print(rx_buffer[3], HEX);
        Serial.print(rx_buffer[4], HEX);
        Serial.print(rx_buffer[5], HEX);
        Serial.print(rx_buffer[6], HEX);
        Serial.print(rx_buffer[7], HEX);
        Serial.print(rx_buffer[8], HEX);
        Serial.println("");
        Serial.print("Send: ");
        Serial.print(tx_buffer[0], HEX);
        Serial.print(tx_buffer[1], HEX);
        Serial.print(tx_buffer[2], HEX);
        Serial.print(tx_buffer[3], HEX);
        Serial.print(tx_buffer[4], HEX);
        Serial.print(tx_buffer[5], HEX);
        Serial.print(tx_buffer[6], HEX);
        Serial.print(tx_buffer[7], HEX);
        Serial.print(tx_buffer[8], HEX);
        Serial.println("");
        return true; // to repeat the action - false to stop
    });
}
