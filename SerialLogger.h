#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>

void initLogger(const int speed);

class SerialLogger {
    public:
        SerialLogger(const int speed);
        ~SerialLogger();

        static void init(const int speed);

        // TODO use initializer list or make a cb with lambdas (did not work at first attempt despite simple c usage...)
        static void logPrint(const String list[], const int size);

        static void logPrint(const String string1);

        static void logPrint(const String string1, const String string2);

        static void logPrint(const String string1, const String string2, const String string3);

        static void logPrint(const String string1, const String string2, const String string3, const String string4);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                      const String string6);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                      const String string6, const String string7);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                      const String string6, const String string7, const String string8);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                      const String string6, const String string7, const String string8, const String string9);

        static void logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                      const String string6, const String string7, const String string8, const String string9, const String string10);
    private:

};

#endif // SERIALLOGGER_H
