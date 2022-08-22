#include "serial_logger.h"
#include <stdarg.h>
#include <stdio.h>

SerialLogger::LOG_LEVEL SerialLogger::logLevel = SerialLogger::LOG_LEVEL::INFO;


void SerialLogger::init(const int speed) {
	Serial.begin(speed);
}

void SerialLogger::init(const int speed, const SerialLogger::LOG_LEVEL logLevel) {
	if (logLevel < LOG_LEVEL::NONE) {
		Serial.begin(speed);
		SerialLogger::logLevel = logLevel;
	}
}

void SerialLogger::trace(const char *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::TRACE) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("TRACE: "));
		log(format, argptr);
		//va_end(argptr);
	}
}

void SerialLogger::debug(const char *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::DEBUG) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("DEBUG: "));
		log(format, argptr);
		//va_end(argptr);
	}
}

void SerialLogger::info(const char *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::INFO) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("INFO:  "));
		log(format, argptr);
		//va_end(argptr);
	}
}

void SerialLogger::warn(const char *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::WARNING) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("WARN:  "));
		log(format, argptr);
		//va_end(argptr);
	}
}

void SerialLogger::error(const char *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::ERROR) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("ERROR: "));
		log(format, argptr);
		//va_end(argptr);
	}
}

void SerialLogger::trace(const __FlashStringHelper *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::TRACE) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("TRACE: "));
		log(format, argptr);
	}
}

void SerialLogger::debug(const __FlashStringHelper *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::DEBUG) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("DEBUG: "));
		log(format, argptr);
	}
}

void SerialLogger::info(const __FlashStringHelper *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::INFO) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("INFO: "));
		log(format, argptr);
	}
}

void SerialLogger::warn(const __FlashStringHelper *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::WARNING) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("WARN: "));
		log(format, argptr);
	}
}

void SerialLogger::error(const __FlashStringHelper *format, ...) {
	if (SerialLogger::logLevel <= SerialLogger::LOG_LEVEL::ERROR) {
		va_list argptr;
		va_start(argptr, format);

		Serial.print(F("ERROR: "));
		log(format, argptr);
	}
}

void SerialLogger::log(const char *format, va_list argptr) {
	bool formatSpecifier = false;
	for (const char *c = format; *c != '\0'; c++) {
		if (*c == '%') {
			formatSpecifier = true;
		} else {
			if (formatSpecifier) {
				if (*c == 's') {
					Serial.print((char *) va_arg(argptr, int));
				} else if (*c == 'd' || *c == 'i') {
					Serial.print(va_arg(argptr, int), DEC);
				} else if (*c == 'x') {
					Serial.print(va_arg(argptr, int), HEX);
				} else if (*c == 'b') {
					Serial.print(va_arg(argptr, int), BIN);
				} else if (*c == 'l') {
					Serial.print(va_arg(argptr, long), DEC);
				} else if (*c == 'c') {
					Serial.print(va_arg(argptr, int));
				} else if (*c == 'f') {
					Serial.print(va_arg(argptr, double), 6);
				}
			} else {
				Serial.print(*c);
			}
			formatSpecifier = false;
		}
	}
	Serial.println();
}

void SerialLogger::log(const __FlashStringHelper *format, va_list argptr) {
	PGM_P p = reinterpret_cast<PGM_P>(format);

	bool formatSpecifier = false;
	for (char c = pgm_read_byte(p); c != '\0'; c = pgm_read_byte(++p)) {
		if (c == '%') {
			formatSpecifier = true;
		} else {
			if (formatSpecifier) {
				if (c == 's') {
					Serial.print((char *) va_arg(argptr, int));
				} else if (c == 'd' || c == 'i') {
					Serial.print(va_arg(argptr, int), DEC);
				} else if (c == 'x') {
					Serial.print(va_arg(argptr, int), HEX);
				} else if (c == 'b') {
					Serial.print(va_arg(argptr, int), BIN);
				} else if (c == 'l') {
					Serial.print(va_arg(argptr, long), DEC);
				} else if (c == 'c') {
					Serial.print(va_arg(argptr, int));
				} else if (c == 'f') {
					Serial.print(va_arg(argptr, double), 6);
				}
			} else {
				Serial.print(c);
			}

			formatSpecifier = false;
		}
	}
	Serial.println();
}

bool SerialLogger::isBelow(const LOG_LEVEL logLevel) {
	return SerialLogger::logLevel <= logLevel;
}
