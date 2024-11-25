#include "sd_logger.h"
#include "identifiers.h"
#include "utils/sd_wrapper.h"
#include "workers/rtc_connector.h"

// e.g. /drives/latest.log
#define TEMP_LOG_NAME_F "%s/latest.log"
// e.g. /drives/2023-11-20_1230.log
#define DATED_LOG_NAME_F "%s/%04d-%02d-%02d_%02d%02d.log"


SdLogger::SdLogger(LocalStorage& config, const LogType log_type) : Handler(), _config(config), _log_type(log_type), _logging_to(""), _is_temp(true), _total(0) {
}

/**
 * @todo
 * Additional header lines for the GNSS flight recorder:
 *  GNSS configuration: constellations and frequency bands
 *  Flight recorder record format and headings
 * Others???
 */
bool SdLogger::activate(bool) {
  // Check SD readiness
  if (!SDInterface::i().can_write_logs()) {
    return false;
  }
  // Create temporary log file
  sprintf(_logging_to, TEMP_LOG_NAME_F, get_dir());
  if (!SDInterface::i().setup_log(get_dir(), _logging_to, true)) {
    return false;
  }
  char header_l2[100];
  char header_l_gnss[100];
  // e.g. # format=1.2.3-zen/drives
  sprintf(header_l2, "%s%d.%d.%d-zen%s", LOG_HEADER_LINE2, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, get_dir());
  snprintf(header_l_gnss, 100, "# hAcc,vAcc,velN,velE,velD,gSpeed,headMot,sAcc,headAcc,invalidLlh");
  bool success = SDInterface::i().log_println(_logging_to, LOG_HEADER_LINE1)
      && SDInterface::i().log_println(_logging_to, header_l2)
      && SDInterface::i().log_println(_logging_to, LOG_HEADER_LINE3)
      && SDInterface::i().log_println(_logging_to, header_l_gnss);

  _is_temp = true;
  _total = 0;
  M5_LOGD("Started new log '%s'.", _logging_to);
  return success;
}

void SdLogger::deactivate() {
  if (_total < MIN_LOG_LINES_FOR_KEEP) {
    // Delete
    M5_LOGD("End logging to '%s', deleting file due to insufficient lines (%d).", _logging_to, _total);
    SDInterface::i().delete_log(_logging_to);
  } else {
    M5_LOGD("End logging to '%s', total of %d lines logged.", _logging_to, _total);
  }
  strcpy(_logging_to, "");
}

int8_t SdLogger::handle_produced_work(const worker_map_t& workers) {
  const auto& log_data = workers.worker<LogAggregator>(k_worker_log_aggregator);
  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
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
        sprintf(new_name, DATED_LOG_NAME_F, get_dir(), rtc_data.year, rtc_data.month, rtc_data.day, rtc_data.hour, rtc_data.minute);
        SDInterface::i().rename_log(_logging_to, new_name);
        strcpy(_logging_to, new_name);
        M5_LOGD("Updated log name to '%s'.", _logging_to);
        _is_temp = false;
      }
    }

    if (log_data->get_data().valid() || settings->get_log_void()) {
      // Line is valid OR we are logging void lines
      if (SDInterface::i().log_println(_logging_to, log_data->get_data().log_string)) {
        // Line written to SD card log file
        _total++;
        return e_handler_data_handled;
      } else {
        // Something went wrong writing to the sd card log file
        return e_handler_error;
      }
    }
  }

  //No data handled
  return e_handler_idle;
}

const char* SdLogger::get_dir() const {
  switch (_log_type) {
    case journal:
      return JOURNAL_LOG_DIRECTORY;
    case drive:
      return DRIVE_LOG_DIRECTORY;
    case survey:
      return SURVEY_LOG_DIRECTORY;
    default:
      return "unknown";
  }
}
