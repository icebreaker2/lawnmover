#ifndef SPI_COMMANDS_H
#define SPI_COMMANDS_H

#include <Arduino.h>
#include <serial_logger.h>

#define COMMAND_FRAME_ID_SIZE 2
#define COMMAND_FRAME_VALUE_SIZE 4
// For the master to receive the nth byte we need to send a n+1 byte
#define COMMAND_SPI_RX_OFFSET 1
#define COMMAND_FRAME_SIZE 9

#define COMMUNICATION_START_SEQUENCE_LENGTH 9

#define LEFT_WHEEL_STEERING_COMMAND (int16_t) 1
#define RIGHT_WHEEL_STEERING_COMMAND (int16_t) 2
#define MOTOR_SPEED_COMMAND (int16_t) 3
#define ENGINE_COMMANDS 3
#define MAX_ID (int16_t) 3

// Could not get templates to work...
class SpiCommand {
    public:
        SpiCommand() = delete;
        SpiCommand(const int id) :
            k_id(id) {
            // nothing to do here...
        };

        int getId() {
            return k_id;
        };

    private:
        const int k_id;
};

class LongSpiCommand: public SpiCommand {
    public:
        using SpiCommand::SpiCommand;
        long interpretBytes(const byte bytes[]) {
            long value;
            memcpy(&value, bytes, sizeof(value));
            return value;
        };
};

class FloatSpiCommand: public SpiCommand {
    public:
        using SpiCommand::SpiCommand;
        static float interpretBytes(const byte bytes[]) {
            float value;
            memcpy(&value, bytes, sizeof(value));
            return value;
        };
};

class IntegerSpiCommand: public SpiCommand {
    public:
        using SpiCommand::SpiCommand;
        static int16_t interpretBytes(const byte bytes[]) {
            int16_t value;
            memcpy(&value, bytes, sizeof(value));
            return value;
        };
};

class BoolSpiCommand: public SpiCommand {
    public:
        using SpiCommand::SpiCommand;
        static  bool interpretBytes(const byte bytes[]) {
            bool value;
            memcpy(&value, bytes, sizeof(value));
            return value;
        };
};

class SpiCommands {
    public:
        template<typename T>
        static bool valueToBytes(const T value, byte *bytes);

        template<typename T>
        static void putCommandToBuffer(const int16_t commandId, const T commandValue, uint8_t *buffer);

        static bool master_interpret_communication(const uint8_t *tx_buffer, const uint8_t *rx_buffer, const long buffer_size);

        template<typename V>
        static bool slave_interpret_command(uint8_t *rx_buffer, bool (*command_callback[])(int16_t, V), const int amount_command_callbacks);

        static bool slave_process_partial_command(bool &synchronized, const uint8_t rx_byte, uint8_t &tx_byte);


        static uint8_t COMMUNICATION_START_SEQUENCE[];

    private:
        static SpiCommand getCommandFrom(const int id);
};

template<typename T>
bool SpiCommands::valueToBytes(const T value, byte *bytes) {
    memcpy(bytes, &value, sizeof(value));
    return true;
}

template<typename T>
void SpiCommands::putCommandToBuffer(const int16_t commandId, const T commandValue, uint8_t *buffer) {
    uint8_t bytes[COMMAND_FRAME_SIZE] = {0};

    SpiCommands::valueToBytes(commandId, bytes);
    SpiCommands::valueToBytes(commandValue, bytes + COMMAND_FRAME_ID_SIZE);
    SpiCommands::valueToBytes((int16_t) - 1, bytes + COMMAND_FRAME_ID_SIZE + COMMAND_FRAME_VALUE_SIZE);
    bytes[COMMAND_FRAME_SIZE - 1] = 0xFF;

    memcpy(buffer, bytes, COMMAND_FRAME_SIZE);
}

/*
    Call if value received by master

    Note: Function-Pointer may differ if request only received. If so, slave must write values to tx_buffer
*/
template<typename V>
bool SpiCommands::slave_interpret_command(uint8_t *rx_buffer, bool (*command_callback[])(int16_t, V), const int amount_command_callbacks) {
    bool valid = false;
    int16_t id = -1;
    byte rxId[COMMAND_FRAME_ID_SIZE];

    for (int id_counter; id_counter < COMMAND_FRAME_ID_SIZE; id_counter++) {
        rxId[id_counter] = rx_buffer[id_counter];
    }

    memcpy(&id, rxId, sizeof(int16_t));
    if (id < 0) {
        SerialLogger::error("Bad Id Received. %d is unknown", id);
    } else if (id > MAX_ID) {
        SerialLogger::warn("Received bad id %d > %d (max)", id, MAX_ID);
    } else {
        V value;
        memcpy(&value, rx_buffer + COMMAND_FRAME_ID_SIZE, sizeof(value));
        for(int i = 0; i < amount_command_callbacks && !valid; i++) {
            valid = (*command_callback[i])(id, value);
        }
    }

    return valid;
}

#endif // SPI_COMMANDS_H
