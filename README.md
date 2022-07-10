# Lawnmover
My own lawnmover setup. I was tired of cutting lawn myself and wanted to learn and do more in electronics and microcontroller developement. 
I had an Arduino left over from past, thus, started with it. 
I know there may be better solutions (e. g. with actual timers on the board like the the STM32) and will check them out in the future.

// TODO add schematics and images/links of parts

## utils
I wanted to share some implementations between the projects. As far as I know modules cannot be shared because an Arduino projects contents get copied to temp directories for building -- excluded any shared implementations whichs reference leads outside the project.
Thus, please copy the contents of the [lawnmover_utils module](lawnmover_utils) to your sketchbook library location. You can find it by:
1. Opening your Arduino IDE
2. Go to File > Preferences > Sketchbook location and copy the location
3. Copy the module to <your-sketchbook-location>/libraries/

It is now available for any project you open. As an alternative, the utils can be zipped and imported using Arduino IDE Sketch > Include Library > Add .ZIP Library...

> **_NOTE:_** I know that a utils module is rather bad (it may mislead you to put every implementation into it rather than concentrating on your business logic implementation, e. g. helpers or helpers helpers with business logic in it^^) but did not know how to solve this in the Arduino world.

## lawnmover_main_core_unit
Is the [Main Core Unit](lawnmover_main_core_unit/README.md) of the lawnmover setup and connected via SPI (master) to slaves (e. g. engine controller board). In addition,
* the main core unit controlls power supply for any slave. If any slave does not respond the power supply gets cut (TODO implement)
* the main core unit provides remote controll access via Bluetooth PS4 Controller. Manual control will always overide and postpone other control commands (e. g. from object detection)

## lawnmover_engines_control_unit
[Lawnmover engine controller](lawnmover_engines_control_unit/README.md) is a SPI-slave to Main Core Unit receiving movement and motor (rotor blades) commands. 
Any received command must be acknowledged by slave within defined time or master will cut power supply.

## lawnmover_distance_control_unit
[Lawnmover distance controller](lawnmover_distance_control_unit/README.md) is a SPI-slave to Main Core Unit sending distances in four directions upon request of MCU. 
Any received command must be answered by slave within defined time or master will cut power supply of the engines and the distance control.

## object_detection
// TODO implement with ultra sonic sensor and camera. 
  * Maybe a raspberry pi is needed for more computational power but mcu should receive states of environment (e. g. object ahead in 2m) from those sensors.
  * Manual controll must override object detection and supsend object detection for some defined time, always!


# TODOs
* Align naming conventions (I am sorry if you have trouble reading the code, I mixed conventions here. Plan is to align with Googles Cpp Guide)
* Add schematics of lawnmovers hardware and circuits
* Add links
* Add docs or choose more descriptive names and types

## License
[License](LICENSE)
