#include "SerialLogger.h"
#include <stdarg.h>
#include <stdio.h>

SerialLogger::LOG_LEVEL SerialLogger::logLevel = SerialLogger::LOG_LEVEL::INFO;


void SerialLogger::init(const int speed, const SerialLogger::LOG_LEVEL logLevel) {
    Serial.begin(speed);
    SerialLogger::logLevel = logLevel;
}

// TODO find a solution without code dupletes....
void SerialLogger::debug(const char * format, ...) {
    if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::DEBUG) {
        // TODO this is rather bad, try to determine length at least and then check if printing in chunks is a better option
        char formatted_string[MAX_FMT_SIZE];

        va_list argptr;
        va_start(argptr, format);
        vsprintf(formatted_string, format, argptr);
        va_end(argptr);

        Serial.println("DEBUG: " + String(formatted_string));
    }
}

void SerialLogger::info(const char * format, ...) {
    if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::INFO) {
        // TODO this is rather bad, try to determine length at least and then check if printing in chunks is a better option
        char formatted_string[MAX_FMT_SIZE];

        va_list argptr;
        va_start(argptr, format);
        vsprintf(formatted_string, format, argptr);
        va_end(argptr);

        Serial.println("INFO:  " + String(formatted_string));
    }
}

void SerialLogger::warn(const char * format, ...) {
    if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::WARNING) {
        // TODO this is rather bad, try to determine length at least and then check if printing in chunks is a better option
        char formatted_string[MAX_FMT_SIZE];

        va_list argptr;
        va_start(argptr, format);
        vsprintf(formatted_string, format, argptr);
        va_end(argptr);

        Serial.println("WARN:  " + String(formatted_string));
    }
}

void SerialLogger::error(const char * format, ...) {
    if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::ERROR) {
        // TODO this is rather bad, try to determine length at least and then check if printing in chunks is a better option
        char formatted_string[MAX_FMT_SIZE];

        va_list argptr;
        va_start(argptr, format);
        vsprintf(formatted_string, format, argptr);
        va_end(argptr);

        Serial.println("ERROR: " + String(formatted_string));
    }
}
