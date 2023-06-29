#ifndef Adafruit_I2CDevice_h
#define Adafruit_I2CDevice_h

#include <Arduino.h>
#include <Wire.h>

///< The class which defines how we will talk to this device over I2C
class Adafruit_I2CDevice {
public:
  Adafruit_I2CDevice(uint8_t addr, TwoWire *theWire = &Wire);
  virtual uint8_t address(void);
  virtual bool begin(bool addr_detect = true);
  virtual void end(void);
  virtual bool detected(void);

  virtual bool read(uint8_t *buffer, size_t len, bool stop = true);
  virtual bool write(const uint8_t *buffer, size_t len, bool stop = true,
        const uint8_t *prefix_buffer = nullptr, size_t prefix_len = 0);
  virtual bool write_then_read(const uint8_t *write_buffer, size_t write_len,
        uint8_t *read_buffer, size_t read_len,
        bool stop = false);
  virtual bool setSpeed(uint32_t desiredclk);

  /*!   @brief  How many bytes we can read in a transaction
   *    @return The size of the Wire receive/transmit buffer */
  virtual size_t maxBufferSize() { return _maxBufferSize; }

protected:
  uint8_t _addr;
  TwoWire *_wire;
  bool _begun;
  size_t _maxBufferSize;
  virtual bool _read(uint8_t *buffer, size_t len, bool stop);
};

#endif // Adafruit_I2CDevice_h
