#ifndef __SD_WRAPPER_H_
#define __SD_WRAPPER_H_

#include "handlers/local_storage.h"

/**
 *  This singleton class provides an interface to check that the SD card is already
 *  connected and operational
 */
class SDInterface {
 public:
  enum sd_error_t {
    SD_ERR_NOT_READY,
    SD_ERR_FILE_OPEN_W_FAIL,
    SD_ERR_FILE_OPEN_R_FAIL
  };

  SDInterface(const SDInterface&) = delete;
  SDInterface& operator=(const SDInterface&) = delete;
  SDInterface(SDInterface&&) = delete;
  SDInterface& operator=(SDInterface&&) = delete;

  /**
   * Singleton
   * @return
   */
  static SDInterface& i() {
    static SDInterface wrapper;
    return wrapper;
  }

  /**
   * @return true if ready
   */
  bool ready();

  /**
   * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
   * @return true if available
   */
  bool begin();

  /**
   * Get device id from SAFEZEN.txt file on the SD card
   * @return device id if available, else 0
   */
  int32_t has_safezen_content();

  /**
   * Read SAFEZEN.txt file contents on SD card to local storage
   * @return true if succeeded
   */
  bool read_safezen_file(const LocalStorage& settings);

  /**
   * Write SAFEZEN.txt file on the SD card from local storage
   * @return true if succeeded
   */
  bool write_safezen_file(const LocalStorage& settings);

 private:
  SDInterface() = default;

  bool _sd_ready = false;
};

#endif  // __SD_WRAPPER_H_
