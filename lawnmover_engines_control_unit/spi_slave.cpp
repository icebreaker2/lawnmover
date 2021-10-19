#include "spi_slave.h"
#include <SPI.h>
#include <Arduino.h>
#include <serial_logger.h>
#include <spi_commands.h>

int pos = 0;
uint8_t id_bytes [COMMAND_FRAME_ID_SIZE];
uint8_t value_bytes [COMMAND_FRAME_VALUE_SIZE];

const int engine_commands_size = ENGINE_COMMANDS * COMMAND_FRAME_SIZE;
int counter = 0;
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
    const uint8_t c = SPDR;
    rx_buffer[counter] = c;
    tx_buffer[counter] = c;

    if (synchronized) {
        if (pos < COMMAND_FRAME_ID_SIZE) {
            id_bytes[pos] = c;
            SPDR = c;
            pos++;
        } else if (pos < COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE) {
            value_bytes[pos - COMMAND_FRAME_ID_SIZE] = c;
            SPDR = c;
            pos++;
        } else if (pos < COMMAND_FRAME_SIZE - COMMAND_SPI_RX_OFFSET) {
            const int &id_index = pos % COMMAND_FRAME_ID_SIZE;
            if (c == 0xFF) {
                SPDR = id_bytes[id_index];
                tx_buffer[counter] = id_bytes[id_index];
                pos++;
            } else {
                //SerialLogger::warn("Received bad ack id request with byte %c on id index %d with position %d. Synchronization lost.", c, id_index, pos);
                SPDR = 0;
                tx_buffer[counter] = 0;

                synchronized = false;
                pos = 0;
                counter = 0;
            }
        } else {
            SPDR = 0;
            tx_buffer[counter] = 0;
            pos = 0;

            const int16_t id = SpiCommands::slave_interpret_command_id(id_bytes);
            const int tx_rx_offset = engine_commands_iterator * COMMAND_FRAME_SIZE;
            const bool valid_command = SpiCommands::slave_interpret_command(id, rx_buffer +  tx_rx_offset + COMMAND_FRAME_ID_SIZE,
                                       tx_buffer + tx_rx_offset + COMMAND_FRAME_ID_SIZE, left_wheel_steering_command_,
                                       right_wheel_steering_command_, motor_speed_command_);
            if (!valid_command) {
                SerialLogger::warn("Did not receive valid command. Cannot interpret value");
            }
            engine_commands_iterator = (engine_commands_iterator + 1) % ENGINE_COMMANDS;
        }
        counter = (counter + 1) % engine_commands_size;
    } else {
        if (rx_buffer[pos] == SpiCommands::COMMUNICATION_START_SEQUENCE[pos]) {
            if (pos == COMMUNICATION_START_SEQUENCE_LENGTH - 1) {
                if (c == 0xFF) {
                    synchronized = true;
                } else {
                    SerialLogger::warn("Bad end-of-sequence-byte received (%x) which should have been %x", c, SpiCommands::COMMUNICATION_START_SEQUENCE[pos]);
                }

                pos = 0;
                counter = 0;
                SPDR = 0;
            } else {
                SPDR = SpiCommands::COMMUNICATION_START_SEQUENCE[pos];
                pos++;
                counter = (counter + 1) % engine_commands_size;
            }
        } else {
            // else restarting synchronization sequence
            synchronized = false;
            pos = 0;
            counter = 0;
            //SerialLogger::warn("Lost synchronization. Received byte %c but expected %c at index %d", c, SpiCommands::COMMUNICATION_START_SEQUENCE[pos], pos);
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
