# Lawnmover Utils
Some utils to install globally and use in a multi-project setup like the Lawnmover.

# Arduino Timer
* Original Timer Lib: https://www.arduino.cc/reference/en/libraries/arduino-timer/ 
* I Modified the every function to take const unsigned long values
* I replaced the C like function pointer of handler_t with the newer C++ std::function allowing stateful functions with this argument by reference!!! 
  This does work for esp32 compilers but not for classic arduino uno....Thus, the library is provided in two versions.
* The timer lib could not be modified w. r. t. std::function for Arduino Uno board + compiler. <functional> is not yet supported. TODO: Check if building yourself will help

# serial_logger
SerialLogger is a "helper" (^^) to provide a static logging mechanism. 
Logging is prevented if selected log level is above Log-Command. 
Methods are based on standard printf usage. Log-Commands are:
* void trace(const char * format, ...)
* void debug(const char * format, ...)
* void info(const char * format, ...)
* void warn(const char * format, ...)
* void error(const char * format, ...)

# spi_commands
* Lookup table and utility service for 9-Byte commands to 
  * Ack each byte send by master
  * Apply protocol to mark end of communication (Byte 9)
    * Where master send 0xFF
    * And slave must send 0x00 in return
  * Make ack id sequence where (byte 7-8)
    * master sends 0xFF 0xFF to request id send with Byte 1 and Byte 2
    * Slave responds with id received from Byte 1 to Byte 2 (e. g. 0x00 0x01)
  * Interpretation of the bytes (e. g. bool, int, long, float)
* Each 9 Byte command conists of 2 byte command id, 4 byte command value and tailing 2 byte command id for acknowledging the command, and the end of the communication as Byte 9

