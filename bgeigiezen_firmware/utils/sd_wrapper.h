#ifndef __SD_WRAPPER_H_
#define __SD_WRAPPER_H_

#include <SD.h>

#include "workers/local_storage.h"

#define MAX_LOG_NAME 255

/**
 *  This singleton class provides an interface to check that the SD card is already
 *  connected and operational
 */
class SDInterface {
 public:
  enum SdStatus {
    e_sd_config_status_not_ready,
    e_sd_config_status_ok,
    e_sd_config_status_config_different_id,
    e_sd_config_status_config_no_content,
    e_sd_config_status_no_config_file,
  };

  enum SdError {
    e_sd_err_not_ready,
    e_sd_err_file_open_w_fail,
    e_sd_err_file_open_r_fail
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
   * Checks sd readiness, will end sd connection if sd has been lost
   * @return true if ready
   */
  bool ready();

  /**
   * Eeturns true if sd was properly set up with device id and all..
   * @return true if ready
   */
  bool can_write_logs() const;

  /**
   * @return last know status
   */
  SdStatus status() const;

  /**
   * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
   * @return true if available
   */
  bool begin();

  /**
   * @return true last write in last 0.5 sec
   */
  bool just_wrote() const;

  /**
   * close SD connection
   */
  void end();

  /**
   */
  bool setup_log(const char* dir, const char* log_name_output, bool clear_on_exist);

  /**
   * Rename log file
   */
  bool rename_log(const char* old_log_name, const char* new_log_name);

  /**
   * Delete log file
   */
  bool delete_log(const char* log_name);

  /**
   */
  bool log_println(const char* log_name, const char* data);

  /**
   * Get device id from SAFEZEN.txt file on the SD card
   * @return device id if available, else 0
   */
  SdStatus has_safezen_content(uint16_t device_id);

  /**
   * Read SAFEZEN.txt file contents on SD card to local storage
   * @return true if succeeded
   */
  bool read_safezen_file_to_settings(LocalStorage& settings);

  /**
   * Write SAFEZEN.txt file on the SD card from local storage
   * @return true if succeeded
   */
  bool write_safezen_file_from_settings(const LocalStorage& settings, bool full = false);

 private:
  SDInterface();

  /**
   * Read SAFEZEN.txt file contents on SD card to local storage
   * @return true if succeeded
   */
  bool read_safezen_file_latest(LocalStorage& settings, File& file);



  SdStatus _status;
  uint32_t _device_id;
  uint32_t _last_read;
  uint32_t _last_write;
  uint32_t _last_try;
};

#endif  // __SD_WRAPPER_H_
