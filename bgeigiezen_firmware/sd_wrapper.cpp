#include "sd_wrapper.h"

#ifdef M5_CORE2
#include <M5Core2.h> // #include <M5Stack.h>
#include <SD.h>
#elif M5_BASIC
#include <M5Stack.h>
#include <SD.h>
#endif

#include "user_config.h"

bool SDInterface::ready() { return _sd_ready; }

/**
 * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
 * @return true if ready
 */
bool SDInterface::begin() {
  if (!_sd_ready) {
    _sd_ready = SD.begin(SD_CS_PIN);
  }
  return _sd_ready;
}

bool SDInterface::get_safecast_content() {
  if (!_sd_ready) {
    return false;
  }

  // open the setup file
  auto setup_file = SD.open("SAFECAST.txt", FILE_READ);

  if (!setup_file) {
    return false;
  }

  // close the setup file
  setup_file.close();

  return true;
}

bool SDInterface::generate_safecast_txt(uint32_t deviceID) {
  File safecast_txt = SD.open("SAFECAST.txt", FILE_WRITE);
  if (!safecast_txt) {
    return false;
  }

  safecast_txt.printf("did=%d", deviceID);
  safecast_txt.close();
  return true;
}
