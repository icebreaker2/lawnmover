#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>

#define MAX_FMT_SIZE 24576

class SerialLogger {
    public:

        enum LOG_LEVEL {
          DEBUG,
          INFO,
          WARNING,
          ERROR
        };

        SerialLogger(const int speed) = delete;

        static void init(const int speed, const LOG_LEVEL logLevel);

        static void debug(const char * format, ...);

        static void info(const char * format, ...);

        static void warn(const char * format, ...);

        static void error(const char * format, ...);

    private:
        static LOG_LEVEL logLevel;

};

#endif // SERIALLOGGER_H
