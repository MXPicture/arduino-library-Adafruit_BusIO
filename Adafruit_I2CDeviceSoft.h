#ifndef Adafruit_I2CDeviceSoft_h
#define Adafruit_I2CDeviceSoft_h

#include <Arduino.h>
#include <Wire.h>
#include <PicI2C.h> // https://github.com/MXPicture/arduino-library-i2c

using pic_i2c::SoftI2C;

#include "Adafruit_I2CDevice.h"

///< The class which defines how we will talk to this device over I2C
class Adafruit_I2CDeviceSoft: public Adafruit_I2CDevice {
public:
    Adafruit_I2CDeviceSoft(uint8_t addr, SoftI2C* theWire)
        : Adafruit_I2CDevice(addr),
        _softWire(theWire) {

#ifdef ARDUINO_ARCH_SAMD
        this->_maxBufferSize = 250; // as defined in Wire.h's RingBuffer
#elif defined(ESP32)
        this->_maxBufferSize = I2C_BUFFER_LENGTH;
#else
        this->_maxBufferSize = 32;
#endif
    };

    inline bool begin(bool addr_detect = true) override {
        this->_softWire;
        this->_softWire->begin();
        this->_begun = true;

        if (addr_detect) {
            return this->detected();
        }
        return true;
    };

    inline void end(void) override {
        // Not all port implement Wire::end(), such as
  // - ESP8266
  // - AVR core without WIRE_HAS_END
  // - ESP32: end() is implemented since 2.0.1 which is latest at the moment.
  // Temporarily disable for now to give time for user to update.
#if !(defined(ESP8266) ||                                                      \
      (defined(ARDUINO_ARCH_AVR) && !defined(WIRE_HAS_END)) ||                 \
      defined(ARDUINO_ARCH_ESP32))

        this->_softWire->end();
        this->_begun = false;
#endif
    };

    inline bool detected(void) override {
        // Init I2C if not done yet
        if (!this->_begun && !this->begin()) return false;

        // A basic scanner, see if it ACK's
        this->_softWire->beginTransmission(_addr);
        return (this->_softWire->endTransmission() == 0);
    };

    inline bool write(const uint8_t* buffer, size_t len, bool stop = true,
        const uint8_t* prefix_buffer = nullptr, size_t prefix_len = 0) override {

        if ((len + prefix_len) > this->maxBufferSize()) {
            // currently not guaranteed to work if more than 32 bytes!
            // we will need to find out if some platforms have larger
            // I2C buffer sizes :/
            return false;
        }

        this->_softWire->beginTransmission(_addr);

        // Write the prefix data (usually an address)
        if ((prefix_len != 0) && (prefix_buffer != nullptr)
            && (this->_softWire->write(prefix_buffer, prefix_len) != prefix_len)) {

            return false;
        }

        // Write the data itself
        if (this->_softWire->write(buffer, len) != len) return false;

        return (this->_softWire->endTransmission(stop) == 0);
    };

    inline bool setSpeed(uint32_t desiredclk) override {
#if defined(__AVR_ATmega328__) ||                                              \
    defined(__AVR_ATmega328P__) // fix arduino core set clock
        // calculate TWBR correctly

        if ((F_CPU / 18) < desiredclk) return false;

        uint32_t atwbr = ((F_CPU / desiredclk) - 16) / 2;
        if (atwbr > 16320) return false;

        if (atwbr <= 255) {
            atwbr /= 1;
            TWSR = 0x0;
        }
        else if (atwbr <= 1020) {
            atwbr /= 4;
            TWSR = 0x1;
        }
        else if (atwbr <= 4080) {
            atwbr /= 16;
            TWSR = 0x2;
        }
        else { //  if (atwbr <= 16320)
            atwbr /= 64;
            TWSR = 0x3;
        }
        TWBR = atwbr;

        return true;
#elif (ARDUINO >= 157) && !defined(ARDUINO_STM32_FEATHER) &&                   \
    !defined(TinyWireM_h)
        this->_softWire->setClock(desiredclk);
        return true;

#else
        (void)desiredclk;
        return false;
#endif
    };

protected:
    SoftI2C* _softWire;

    inline bool _read(uint8_t* buffer, size_t len, bool stop) override {
#if defined(TinyWireM_h)
        size_t recv = _softWire->requestFrom((uint8_t)_addr, (uint8_t)len);
#elif defined(ARDUINO_ARCH_MEGAAVR)
        size_t recv = _softWire->requestFrom(_addr, len, stop);
#else
        size_t recv = _softWire->requestFrom((uint8_t)_addr, (uint8_t)len, (uint8_t)stop);
#endif

        // Not enough data available to fulfill our obligation!
        if (recv != len) return false;

        for (uint16_t i = 0; i < len; i++) {
            buffer[i] = _softWire->read();
        }

        return true;
    };
};

#endif // Adafruit_I2CDeviceSoft_h
