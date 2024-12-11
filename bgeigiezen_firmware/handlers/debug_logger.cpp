#include "debug_logger.h"
#include "identifiers.h"
#include "utils/sd_wrapper.h"
#include "workers/navsat_collector.h"
#include "workers/rtc_connector.h"

// e.g. /debug/gps-latest.log
#define TEMP_LOG_NAME_F "%s/%s-latest.log"
// e.g. /debug/gps-2023-11-20_1230.log
#define DATED_LOG_NAME_F "%s/%s-%04d-%02d-%02d_%02d%02d.log"


/// BaseDebugLogger

/*************  ✨ Codeium Command ⭐  *************/
/**
 * @brief Constructor for BaseDebugLogger
 *
 * @param config Reference to the LocalStorage worker
 * @param logging_name C string of the logging name (e.g. "gps", "survey", etc.)
 *
 * Note that this constructor is protected, meaning that it can only be called
 * from the constructors of derived classes.
 */
/******  e0f75267-1cb5-4c2c-a952-0f649ea223b8  *******/
BaseDebugLogger::BaseDebugLogger(LocalStorage& config, const char* logging_name) : Handler(), _config(config), _logging_name(""), _logging_to(""), _is_temp(true), _total(0) {
  strcpy(_logging_name, logging_name);
}

/*************  ✨ Codeium Command ⭐  *************/
/**
 * Activate the debug logger. If the SD card is not ready for writing, then
 * immediately return false. Otherwise, create a new temporary log file
 * and return true.
 *
 * @param retry If true, then try to activate the logger even if the SD
 * card is not ready. If false, then immediately return false if the SD
 * card is not ready.
 *
 * @return true if the logger was activated, false otherwise.
 */
/******  8e45a78d-1bf4-4b27-b39a-9478339104b8  *******/
bool BaseDebugLogger::activate(bool) {
  // Check SD readiness
  if (!SDInterface::i().can_write_logs()) {
    return false;
  }
  // Create temporary log file
  sprintf(_logging_to, TEMP_LOG_NAME_F, DEBUG_LOG_DIRECTORY, _logging_name);
  if (!SDInterface::i().setup_log(DEBUG_LOG_DIRECTORY, _logging_to, true)) {
    return false;
  }
  _is_temp = true;
  _total = 0;
  M5_LOGD("Started new debug log '%s'.", _logging_to);
  return true;
}

/*************  ✨ Codeium Command ⭐  *************/
  /**
   * Deactivate the debug logger. If the number of lines logged is less than
   * MIN_LOG_LINES_FOR_KEEP, then delete the log file. Otherwise, log the
   * total number of lines logged. Clear the log file name.
   */
/******  41f3248b-fdec-4bcb-a760-72e16da28360  *******/
void BaseDebugLogger::deactivate() {
  if (_total < MIN_LOG_LINES_FOR_KEEP) {
    // Delete
    M5_LOGD("End logging to '%s', deleting file due to insufficient lines (%d).", _logging_to, _total);
    SDInterface::i().delete_log(_logging_to);
  } else {
    M5_LOGD("End logging to '%s', total of %d lines logged.", _logging_to, _total);
  }
  strcpy(_logging_to, "");
}

/*************  ✨ Codeium Command ⭐  *************/
  /**
   * Handle produced work. If the log aggregator has new data, and the sd card is
   * ready, then write the line to the log file. If the log file is currently
   * temporary, then use the rtc to rename the log file. If the write succeeds,
   * then increment _total and return e_handler_data_handled. If there is no new
   * data, then return e_handler_idle.
   *
   * @param workers
   * @return
   */
/******  67e2355c-5e52-4edc-9f0e-16f42b029155  *******/
int8_t BaseDebugLogger::handle_produced_work(const worker_map_t& workers) {
  const auto& log_data = workers.worker<LogAggregator>(k_worker_log_aggregator);
  if (log_data->is_fresh()) {
    if (!SDInterface::i().ready()) {
      M5_LOGD("Abrupt stop logging '%s', sd card not ready.", _logging_to);
      return e_handler_error;
    }
    if (_is_temp) {
      // Check RTC for time to rename log file
      const auto& rtc_data = workers.worker<DateTimeProvider>(k_worker_rtc_connector)->get_data();
      if (rtc_data.valid) {
        // Renaming the log file
        char new_name[LOG_FILENAME_SIZE];
        sprintf(new_name, DATED_LOG_NAME_F, DEBUG_LOG_DIRECTORY, _logging_name, rtc_data.year, rtc_data.month, rtc_data.day, rtc_data.hour, rtc_data.minute);
        SDInterface::i().rename_log(_logging_to, new_name);
        strcpy(_logging_to, new_name);
        M5_LOGD("Updated log name to '%s'.", _logging_to);
        _is_temp = false;
      }
    }

    if (write_line(workers)) {
      ++_total;
      return e_handler_data_handled;
    }
  }

  //No data handled
  return e_handler_idle;
}


/// GpsDebugLogger

/*************  ✨ Codeium Command ⭐  *************/
/**
 * @brief Write a line to the debug log using the latest GPS and NavSat data
 *
 * @param workers The current worker map
 *
 * @return true if the line was written, false otherwise
 */
/******  387bb3b6-2151-4835-9dfa-79a5e965ae24  *******/
bool GpsDebugLogger::write_line(const worker_map_t& workers) {
  const auto& gps = workers.worker<GpsConnector>(k_worker_navsat_collector);
  const auto& navsat = workers.worker<NavsatCollector>(k_worker_navsat_collector);

  if (gps->active() && navsat->active()) {
    char log_string[200];
    sprintf(log_string, "format here"); // Format data from gps / navsat workers

    return SDInterface::i().log_println(_logging_to, log_string);
  }
  return false;
}
