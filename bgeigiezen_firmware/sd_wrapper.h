#ifndef __SD_WRAPPER_HPP__
#define __SD_WRAPPER_HPP__

#include <stdint.h>

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
   * Get contents from SAFECAST.txt file on the SD card
   * @return true if success
   */
  bool get_safecast_content();

  /**
   * Get contents from SAFECAST.txt file on the SD card
   * @return true if success
   */
  bool generate_safecast_txt(uint32_t deviceID);

 private:
  SDInterface() = default;

  bool _sd_ready = false;
};

#endif  // __SD_WRAPPER_HPP__
