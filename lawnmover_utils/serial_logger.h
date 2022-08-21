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
		ERROR,
		NONE
	};

	SerialLogger() = delete;

	static void init(const int speed);

	static void init(const int speed, const LOG_LEVEL logLevel);

	static void trace(const char *format, ...);

	static void debug(const char *format, ...);

	static void info(const char *format, ...);

	static void warn(const char *format, ...);

	static void error(const char *format, ...);

	static void trace(const __FlashStringHelper *format, ...);

	static void debug(const __FlashStringHelper *format, ...);

	static void info(const __FlashStringHelper *format, ...);

	static void warn(const __FlashStringHelper *format, ...);

	static void error(const __FlashStringHelper *format, ...);

	static bool isBelow(const LOG_LEVEL logLevel);

private:
	static LOG_LEVEL logLevel;

	static void log(const char *format, va_list argptr);

	static void log(const __FlashStringHelper *format, va_list argptr);
};

#endif // SERIALLOGGER_H
