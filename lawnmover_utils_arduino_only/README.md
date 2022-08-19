# Lawnmover Utils
Some utils to install globally and use in a multi-project setup like the Lawnmover which are for arduino procjects only. 
This is needed to prevent esp32 or other hardware to bother from wrong implementations.

## Installation
Given the arduino IDE and compilers (same for esp32), we cannot include subdirectories sources or even modules on the same directory layer of the module to compile.
Unfortunately, the Arduino IDE does not follow symlinks of any kind.
Thus, please copy this directory to your sketchbook library location. You can find it by:
1. Opening your Arduino IDE
2. Go to File > Preferences > Sketchbook location and copy the location
3. Copy the module to <your-sketchbook-location>/libraries/

It is now available for any project you open.

# spi slave
* An Arduino interrupt routine implementation for SPI with minimal interface
* Is not thread-safe
* Is a Singleton (but not enforced, not reentrant). There can be only one SPI Interrupt routine per application.
* Does not work with ESP32 boards due to compilation issues using ISR

# watchdog
* A watchdog to cut critical loads from power or just enable some fallback measurements if any "event" happens
* An event may be that after some elapsed time a counter was not incremented enough. Not reaching the threshold may then result in safety procedures called to disable some pins, loads, or others.
* Does not work with ESP32 boards due to compilation issues using the arduino timer syntax
