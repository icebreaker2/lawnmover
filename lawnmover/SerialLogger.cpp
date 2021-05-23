#include "SerialLogger.h"

void initLogger(const int speed) {
  Serial.begin(speed);
}

SerialLogger::SerialLogger(const int speed) {
    init(speed);
}

SerialLogger::~SerialLogger() {
    // nothing to do ...
}

static void SerialLogger::init(const int speed) {
  Serial.begin(speed);
}

static void SerialLogger::logPrint(const String list[], const int size) {
    for (int i = 0; i < size; i++) {
        Serial.print(list[i]);
    }
    Serial.println("");
}

static void SerialLogger::logPrint(const String string1) {
    Serial.println(string1);
}

static void SerialLogger::logPrint(const String string1, const String string2) {
    Serial.print(string1);
    Serial.println(string2);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.println(string3);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.println(string4);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.println(string5);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                            const String string6) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.print(string5);
    Serial.println(string6);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                            const String string6, const String string7) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.print(string5);
    Serial.print(string6);
    Serial.println(string7);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                            const String string6, const String string7, const String string8) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.print(string5);
    Serial.print(string6);
    Serial.print(string7);
    Serial.println(string8);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                            const String string6, const String string7, const String string8, const String string9) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.print(string5);
    Serial.print(string6);
    Serial.print(string7);
    Serial.print(string8);
    Serial.println(string9);
}

static void SerialLogger::logPrint(const String string1, const String string2, const String string3, const String string4, const String string5,
                            const String string6, const String string7, const String string8, const String string9, const String string10) {
    Serial.print(string1);
    Serial.print(string2);
    Serial.print(string3);
    Serial.print(string4);
    Serial.print(string5);
    Serial.print(string6);
    Serial.print(string7);
    Serial.print(string8);
    Serial.print(string9);
    Serial.println(string10);
}