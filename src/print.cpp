#include "print.h"

#include "HardwareSerial.h"

void print(const char* c) {
    Serial.println(c);
}
