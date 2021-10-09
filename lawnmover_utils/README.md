# utils

## Notes:
* Timer Lib: 
  * I Modified the every function to take const unsigned long values
  * I replaced the C like function pointer of handler_t with the newer C++ std::function allowing stateful functions with this argument by reference!!! 
    This does work for esp32 compilers but not for classic arduino uno....Thus, the library is provided in two versions.
  * The timer lib could not be modified w. r. t. std::function for Arduino Uno board + compiler. <functional> is not yet supported. TODO: Check if building yourself will help

* SerialLogger is a "helper" (^^) to provide a logging mechanism via singleton. Is disabled if not initialized in debug mode.

