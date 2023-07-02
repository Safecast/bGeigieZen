#ifndef __SD_WRAPPER_HPP__
#define __SD_WRAPPER_HPP__
/*
 *  This class provides an interface to check that the SD card is already
 *  connected and operational
 */
#include <M5Core2.h> // #include <M5Stack.h>

#include <config.hpp>

enum sd_error_t {
  SD_ERR_NOT_READY,
  SD_ERR_FILE_OPEN_W_FAIL,
  SD_ERR_FILE_OPEN_R_FAIL
};

class SDWrapper {
 private:
  bool _sd_ready = false;

 public:
  // We note that for the ESP32, the SD library is robust to multiple call
  // to the begin() function
  // https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/src/SD.cpp#L27
  SDWrapper() {}
  bool ready() { return _sd_ready; }
  bool begin() {
    if (!_sd_ready) {
      _sd_ready = SD.begin(SD_CS_PIN);
    }
    return _sd_ready;
  }
};

#endif  // __SD_WRAPPER_HPP__
