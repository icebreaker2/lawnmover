# Lawnmover Utils
Some utils to install globally and use in a multi-project setup like the Lawnmover which are for arduino procjects only. 
This is needed to prevent esp32 or other hardware to bother from wrong implementations.

# spi slave
* An Arduino interrupt routine implementation for SPI with minimal interface
* Is not thread-safe
* Is a Singleton (but not enforced, not reentrant). There can be only one SPI Interrupt routine per application.
* Does not work with ESP32 boards due to compilation issues using ISR

# watchdog
* A watchdog to cut critical loads from power or just enable some fallback measurements if any "event" happens
* An event may be that after some elapsed time a counter was not incremented enough. Not reaching the threshold may then result in safety procedures called to disable some pins, loads, or others.
* Does not work with ESP32 boards due to compilation issues using the arduino timer syntax
