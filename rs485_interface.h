#pragma once

class RS485Interface {
public:
    virtual void begin() = 0;
    virtual int available() = 0;
    virtual int read_byte() = 0;
    virtual int write_bytes(const void* data, int size) = 0;
    virtual void flush() = 0;
    virtual void enable_receiver_mode() = 0;
    virtual void enable_sender_mode() = 0;
};

#ifdef ARDUINO

#include <Arduino.h>
#include <HardwareSerial.h>

class ArduinoRS485Interface: public RS485Interface {
public:
    ArduinoRS485Interface(HardwareSerial& hw_serial, int de_pin):
        _hw_serial(hw_serial),
        _de_pin(de_pin) {}

    void begin() {
       _hw_serial.begin(19200, SERIAL_8N1);
	    pinMode(_de_pin, OUTPUT);
        enable_receiver_mode();
    }

    int available() {
        return _hw_serial.available();
    }

    int read_byte() {
        return _hw_serial.read();
    }

    int write_bytes(const void* data, int size) {
        return _hw_serial.write((const uint8_t*)data, size);
    }

    void flush() {
        return _hw_serial.flush();
    }

    void enable_receiver_mode() {
	    digitalWrite(_de_pin, LOW);
    }

    void enable_sender_mode() {
	    digitalWrite(_de_pin, HIGH);
    }

private:
    HardwareSerial &_hw_serial;
    int _de_pin;
};

#endif
