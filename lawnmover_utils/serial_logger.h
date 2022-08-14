#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>

class SerialLogger {
    public:

        enum LOG_LEVEL {
            TRACE,
            DEBUG,
            INFO,
            WARNING,
            ERROR
        };

        SerialLogger() = delete;

        static void init(const int speed);

        static void init(const int speed, const LOG_LEVEL logLevel);

        static void trace(const char * format, ...);

        static void debug(const char * format, ...);

        static void info(const char * format, ...);

        static void warn(const char * format, ...);

        static void error(const char * format, ...);

		static bool is(const LOG_LEVEL logLevel);

    private:
        static LOG_LEVEL logLevel;

        static void log(const char *format, va_list argptr);
};

#endif // SERIALLOGGER_H
