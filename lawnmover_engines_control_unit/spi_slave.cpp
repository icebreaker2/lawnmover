#include "spi_slave.h"
#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>
#include <spi_commands.h>


const int engine_commands_size = ENGINE_COMMANDS * COMMAND_FRAME_SIZE;
int buffer_counter = 0;
int engine_commands_iterator = 0;
uint8_t rx_buffer[engine_commands_size];
uint8_t tx_buffer[engine_commands_size];

bool synchronized = false;

bool (*left_wheel_steering_command_)(int16_t) = nullptr;
bool (*right_wheel_steering_command_)(int16_t) = nullptr;
bool (*motor_speed_command_)(int16_t) = nullptr;

SpiSlave::SpiSlave(const int sck_pin, const int miso_pin, const int mosi_pin, const int ss_pin,
                   bool (*left_wheel_steering_command)(int16_t), bool (*right_wheel_steering_command)(int16_t),
                   bool (*motor_speed_command)(int16_t)) {
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

    left_wheel_steering_command_ = left_wheel_steering_command;
    right_wheel_steering_command_ = right_wheel_steering_command;
    motor_speed_command_ = motor_speed_command;
}

/**
    SPI interrupt routine
    Note: Printing consumes too much time. Slave does not respond in time.
*/
ISR (SPI_STC_vect) {
    const uint8_t rx_byte = SPDR;
    uint8_t tx_byte = 0;
    const bool previously_synchronized = synchronized;

    const bool full_command_or_synchronized = SpiCommands::slave_process_partial_command(synchronized, rx_byte, tx_byte);
    SPDR = tx_byte;
    rx_buffer[buffer_counter] = rx_byte;
    tx_buffer[buffer_counter] = tx_byte;

    if (full_command_or_synchronized && previously_synchronized) {
        const int tx_rx_offset = engine_commands_iterator * COMMAND_FRAME_SIZE;
        const int16_t id = SpiCommands::slave_interpret_command_id(rx_buffer + tx_rx_offset);
        const bool valid_command = SpiCommands::slave_interpret_command(id, rx_buffer +  tx_rx_offset + COMMAND_FRAME_ID_SIZE,
                                   tx_buffer + tx_rx_offset + COMMAND_FRAME_ID_SIZE, left_wheel_steering_command_,
                                   right_wheel_steering_command_, motor_speed_command_);
        engine_commands_iterator = (engine_commands_iterator + 1) % ENGINE_COMMANDS;
        buffer_counter = engine_commands_iterator * COMMAND_FRAME_SIZE;
        if (!valid_command) {
            SerialLogger::warn("Did not receive valid command. Cannot interpret value");
        }
    } else {
        if (previously_synchronized) {
            buffer_counter = (buffer_counter + 1) % COMMAND_FRAME_SIZE;
        } else {
            if (synchronized) {
                engine_commands_iterator = 0;
                buffer_counter = 0;
            } else {
                buffer_counter = (buffer_counter + 1) % COMMAND_FRAME_SIZE;
            }
        }
    }
}  // end of interrupt service routine (ISR) SPI_STC_vect

void SpiSlave::addSlavePrinting(Timer<> &timer, const int interval) {
    timer.every(interval, [](void*) -> bool {
        Serial.print("RxBufferInput:");
        for (long i = 0; i < engine_commands_size; i += 1) {
            if (i % COMMAND_FRAME_SIZE == 0) {
                Serial.print(" ");
            }
            Serial.print(rx_buffer[i], HEX);
        }
        Serial.println();


        Serial.print("TxBufferInput:");
        for (long i = 0; i < engine_commands_size; i += 1) {
            if (i % COMMAND_FRAME_SIZE == 0) {
                Serial.print(" ");
            }
            Serial.print(tx_buffer[i], HEX);
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
