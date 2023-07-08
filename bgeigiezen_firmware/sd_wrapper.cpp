#include "sd_wrapper.h"

#ifdef M5_CORE2
#include <M5Core2.h> // #include <M5Stack.h>
#include <SD.h>
#elif M5_BASIC
#include <M5Stack.h>
#include <SD.h>
#endif

#include "user_config.h"

bool SDWrapper::ready() { return _sd_ready; }

/**
 * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
 * @return true if ready
 */
bool SDWrapper::begin() {
  if (!_sd_ready) {
    _sd_ready = SD.begin(SD_CS_PIN);
  }
  return _sd_ready;
}
