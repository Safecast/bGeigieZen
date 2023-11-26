#include "sd_logger.h"
#include "identifiers.h"
#include "utils/sd_wrapper.h"
#include "workers/rtc_connector.h"

// e.g. /drives/latest.txt
#define TEMP_LOG_NAME_F "%s/latest.txt"
// e.g. /drives/2023-11-20_1230.txt
#define DATED_LOG_NAME_F "%s/%d-%d-%d_%d%d.txt"


SdLogger::SdLogger(LocalStorage& config, const LogType log_type) : Handler(), _config(config), _log_type(log_type), _logging_to(""), _is_temp(true) {
}

bool SdLogger::activate(bool) {
  // Check SD readiness
  if (!SDInterface::i().ready()) {
    return false;
  }
  // Create temporary log file
  sprintf(_logging_to, TEMP_LOG_NAME_F, get_dir());
  if (!SDInterface::i().setup_log(get_dir(), _logging_to, true)) {
    return false;
  }
  char header_l2[100];
  // e.g. # format=1.2.3-drives
  sprintf(header_l2, "%s%d.%d.%d-%s", LOG_HEADER_LINE2, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, get_dir());
  bool success = SDInterface::i().log_println(_logging_to, LOG_HEADER_LINE1)
      && SDInterface::i().log_println(_logging_to, header_l2)
      && SDInterface::i().log_println(_logging_to, LOG_HEADER_LINE3);

  _is_temp = true;
  DEBUG_PRINTF("Started new log '%s'.\n", _logging_to);
  return success;
}

void SdLogger::deactivate() {
  DEBUG_PRINTF("End logging to '%s'.\n", _logging_to);
  strcpy(_logging_to, "");
}

int8_t SdLogger::handle_produced_work(const worker_map_t& workers) {
  const auto& log_data = workers.worker<LogAggregator>(k_worker_log_aggregator);
  if(log_data->is_fresh()) {
    if (_is_temp) {
      const auto& rtc_data = workers.worker<RtcConnector>(k_worker_rtc_connector)->get_data();
      if (rtc_data.valid) {
        // Renaming the log file
        char new_name[LOG_FILENAME_SIZE];
        sprintf(new_name, DATED_LOG_NAME_F, get_dir(), rtc_data.year, rtc_data.month, rtc_data.day, rtc_data.hour, rtc_data.minute);
        SDInterface::i().rename_log(_logging_to, new_name);
        strcpy(_logging_to, new_name);
        DEBUG_PRINTF("Updated log name to '%s'.\n", _logging_to);
        _is_temp = false;
      }
    }

    SDInterface::i().log_println(_logging_to, log_data->get_data().log_string);
  }

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
      return "";
  }
}
