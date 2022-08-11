#include "spi_slave.h"

#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>
#include <spi_commands.h>


const int _engine_commands_size = ENGINE_COMMANDS * COMMAND_FRAME_SIZE;
int _buffer_counter = 0;
int _engine_commands_indicator = 0;
uint8_t _rx_buffer[_engine_commands_size];
uint8_t _tx_buffer[_engine_commands_size];

bool synchronized = false;

const int amount_commands = 3;
bool (*_commands[amount_commands])(int16_t, int16_t);

SpiSlave::SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
                   bool (*left_wheel_steering_command)(int16_t, int16_t), 
                   bool (*right_wheel_steering_command)(int16_t, int16_t),
                   bool (*motor_speed_command)(int16_t, int16_t)) {
                    // TODO: make this arguments the commands list. This object does not need to know its commands
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

    _commands[0] = left_wheel_steering_command;
    _commands[1] = right_wheel_steering_command;
    _commands[2] = motor_speed_command;
}

/**
    SPI interrupt routine
    Note: Printing consumes too much time. Slave does not respond in time.
*/
ISR (SPI_STC_vect) {
    const uint8_t rx_byte = SPDR;
    uint8_t tx_byte = 0;
    const bool previously_synchronized = synchronized;

    const bool full_commandor_synchronized = SpiCommands::slave_process_partial_command(synchronized, rx_byte, tx_byte);
    SPDR = tx_byte;
    _rx_buffer[_buffer_counter] = rx_byte;
    _tx_buffer[_buffer_counter] = tx_byte;

    if (full_commandor_synchronized && previously_synchronized) {
        const int tx_rx_offset = _engine_commands_indicator * COMMAND_FRAME_SIZE;
        const bool command_interpreted = SpiCommands::slave_interpret_command(_rx_buffer +  tx_rx_offset, _commands, amount_commands);
        _engine_commands_indicator = (_engine_commands_indicator + 1) % ENGINE_COMMANDS;
        _buffer_counter = _engine_commands_indicator * COMMAND_FRAME_SIZE;
        if (!command_interpreted) {
            SerialLogger::warn("Did not receive valid command. Cannot interpret value");
        }
    } else {
        if (previously_synchronized) {
            _buffer_counter = (_buffer_counter + 1) % _engine_commands_size;
        } else {
            if (synchronized) {
                _engine_commands_indicator = 0;
                _buffer_counter = 0;
            } else {
                _buffer_counter = (_buffer_counter + 1) % _engine_commands_size;
            }
        }
    }
}  // end of interrupt service routine (ISR) SPI_STC_vect

void SpiSlave::addSlavePrinting(Timer<> &timer, const int interval) {
    timer.every(interval, [](void*) -> bool {
        Serial.print("RxBufferInput:");
        for (long i = 0; i < _engine_commands_size; i += 1) {
            if (i % COMMAND_FRAME_SIZE == 0) {
                Serial.print(" ");
            }
            Serial.print(_rx_buffer[i], HEX);
        }
        Serial.println();


        Serial.print("TxBufferInput:");
        for (long i = 0; i < _engine_commands_size; i += 1) {
            if (i % COMMAND_FRAME_SIZE == 0) {
                Serial.print(" ");
            }
            Serial.print(_tx_buffer[i], HEX);
        }
        Serial.println();
        if (synchronized) {
            SerialLogger::info("Slave is synchronized");
        } else {
            SerialLogger::info("Slave is NOT synchronized");
        }

        return true; // to repeat the action - false to stop
    });
}
