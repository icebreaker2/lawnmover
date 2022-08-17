# Lawnmover
My own lawnmover setup. I was tired of cutting lawn myself and wanted to learn and do more in electronics and microcontroller development. 
I had an Arduino left over from past, thus, started with it. In time, I added further boards for distance control or radio support.
I know there may be better solutions (e. g. with actual timers on the board like the the STM32) and will check them out in the future.

## schematics, circuits and parts
// TODO add schematics and images/links of parts

## utils
Some utils used by various different other modules in this project. 
These utils are [lawnmover_utils module](lawnmover_utils) and [lawnmover_utils_arduino_only](lawnmover_utils_arduino_only). 
Please follow the installation instructions for these models.

> **_NOTE:_** I know that a utils module is rather bad (it may mislead you to put every implementation into it rather than concentrating on your business logic implementation, e. g. helpers or helpers helpers with business logic in it^^) but I did not want to copy-paste a vast amount of modules per domain.

## lawnmover_main_core_unit
Is the [Main Core Unit](lawnmover_main_core_unit/README.md) of the lawnmover setup and connected via SPI (master) to slaves (e. g. engine controller board). In addition,
* the main core unit controls hardware reset pins for any slave. If any slave does not respond the slave gets restarted
* the main core unit uses the [Lawnmover robo pilot](lawnmover_robo_pilot/README.md) implementations to determine the next movement
* the main core unit provides remote control access via Bluetooth PS4 Controller. Manual control will always override and postpone other control commands from robo pilot

## lawnmover_engines_control_unit
[Lawnmover engine controller](lawnmover_engines_control_unit/README.md) is a SPI-slave to Main Core Unit receiving movement and motor (rotor blades) commands. 
Any received command must be acknowledged by slave within defined time or master will initiate a restart via HW restart pin.

## lawnmover_distance_control_unit
[Lawnmover distance controller](lawnmover_distance_control_unit/README.md) is a SPI-slave to Main Core Unit sending distances in multiple directions upon request of MCU. 
Any received command must be answered by slave within defined time or master will initiate a restart via HW restart pin.

## object_detection
// TODO implement with ultra sonic sensor and camera. 
  * Maybe a raspberry pi is needed for more computational power but mcu should receive states of environment (e. g. object ahead in 2m) from those sensors.
  * Manual controll must override object detection and supsend object detection for some defined time, always!

## lawnmover_robo_pilot
[Lawnmover robo pilot](lawnmover_robo_pilot/README.md) contains the implementation for various smart decision making algorithms/auto pilots.  
Please follow the installation instructions for these models.

# TODOs
* Align naming conventions (I am sorry if you have trouble reading the code, I mixed conventions here. Plan is to align with Googles Cpp Guide)
* Add schematics of lawnmovers hardware and circuits
* Add links
* Add docs or choose more descriptive names and types
* Add 3D sketches for printing case and handmade parts
* Add per module SW architecture documentation

## License
[License](LICENSE)
