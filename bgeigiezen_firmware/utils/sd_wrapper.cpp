#include "sd_wrapper.h"

#ifdef M5_CORE2
#include <M5Core2.h> // #include <M5Stack.h>
#include <SD.h>
#elif M5_BASIC
#include <M5Stack.h>
#include <SD.h>
#endif

#include "user_config.h"

constexpr char sd_config_device_id_f[] = "did=%d";



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

int32_t SDInterface::has_safezen_content() {
  if (!_sd_ready) {
    return -1;
  }

  if (!SD.exists(SETUP_FILENAME)) {
    return 0;
  }

  // open the setup file
  auto setup_file = SD.open(SETUP_FILENAME, FILE_READ);

  if (!setup_file) {
    return 0;
  }


  // close the setup file
  setup_file.close();

  return true;
}

bool SDInterface::read_safezen_file(const LocalStorage& settings) {
  if (SD.exists(SETUP_FILENAME)) {
    SD.remove(SETUP_FILENAME);
  }
  File safecast_txt = SD.open(SETUP_FILENAME, FILE_WRITE);
  if (!safecast_txt) {
    return false;
  }

  safecast_txt.printf("did=%d", settings.get_device_id());
  safecast_txt.close();
  return true;
}

bool SDInterface::write_safezen_file(const LocalStorage& settings) {
  if (SD.exists(SETUP_FILENAME)) {
    SD.remove(SETUP_FILENAME);
  }
  File safecast_txt = SD.open(SETUP_FILENAME, FILE_WRITE);
  if (!safecast_txt) {
    return false;
  }

  safecast_txt.printf(sd_config_device_id_f, settings.get_device_id());
  safecast_txt.close();
  return true;
}
