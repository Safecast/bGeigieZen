#ifndef __SD_WRAPPER_HPP__
#define __SD_WRAPPER_HPP__

/**
 *  This singleton class provides an interface to check that the SD card is already
 *  connected and operational
 */
class SDWrapper {
 public:
  enum sd_error_t {
    SD_ERR_NOT_READY,
    SD_ERR_FILE_OPEN_W_FAIL,
    SD_ERR_FILE_OPEN_R_FAIL
  };

  SDWrapper(const SDWrapper&) = delete;

  SDWrapper& operator=(const SDWrapper&) = delete;

  SDWrapper(SDWrapper&&) = delete;

  SDWrapper& operator=(SDWrapper&&) = delete;

  /**
   * Singleton
   * @return
   */
  static SDWrapper& i() {
    static SDWrapper wrapper;
    return wrapper;
  }

  bool ready();

  /**
   * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
   * @return true if ready
   */
  bool begin();

 private:
  SDWrapper() = default;

  bool _sd_ready = false;

};

#endif  // __SD_WRAPPER_HPP__
