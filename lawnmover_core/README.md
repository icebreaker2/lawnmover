# Lawnmover

## Notes:
* ESP for arduino: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
* Use a ESP32 module for bluetooth conneciton:
* For that, you may need to manually install driver 
  * The CH340 is the driver in question
  * https://chewett.co.uk/blog/1049/wemos-d1-mini-not-recognised-windows-no-com-port/
  * https://www.wemos.cc/en/latest/tutorials/d1/get_started_with_arduino_d1.html
* Choose Esp32vn IoT Uno as Board for AZ Delivery D1 R32 Esp32 board

* PS3 controller connection:
  * https://github.com/user-none/sixaxispairer (get to know ps3 mac address)
  * https://github.com/jvpernis/esp32-ps3 (connect esp32 board with ps3 controller)

* Ps4 controller connection:
  * use your ps4 do get to know the controllers master mac address (the ps4 address)
  * or use a tool like sixaxispairer to get to know it or even change the mac address to your needs
  * Download and apply: https://github.com/aed3/PS4-esp32
  * See https://github.com/jvpernis/esp32-ps3/commit/dbcea43a8a246fef6e07b613eee0ea6973c71963 for trouble shooting wrong idf reference
  * Important: You may need to install if it is not shipped with the board registraton of esp32 module via Arduino IDE --> ESP-IDF (https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html )
  * Enter your mac address recently fetched (or set)


## Power-Consumption:
* Arduino (Battery Active + Motor Active + LED Active Pin + LED + PWM + MotorCtrl connected)
  * At 8V 
  *

## Motor needs:
* 116mA at 6v (laut Angabe)
* 126mA bei 9v (selbst gemessen)
* Should be (extapolated)
  max 140-150 mA at 12v per motor
  max 300 mA. at 19v  per motor
// TODO update to newer motors


