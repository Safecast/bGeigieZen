#include "battery_logger.h"

#include <M5Unified.h>
#include <SD.h>
#include <SPI.h>

#include "utils/sd_wrapper.h"

#include "workers/local_storage.h"
#include "workers/battery_indicator.h"
#include "workers/rtc_connector.h"

#define BATT_LOG_DIR "/battery_logs"
#define BATT_LOG_FILE_PREFIX "battery_"
#define BATT_LOG_TEMP_FILENAME BATT_LOG_DIR "/latest_batt.csv"
#define BATT_LOG_HEADER "device_id,uptime_s,voltage_v,percentage,current_ma"
#define BATT_MIN_LOG_LINES 1  // Minimum number of lines to keep the log file

BatteryLogger::BatteryLogger()
    : ProcessWorker<BatteryLogEntry>(BATTERY_LOG_INTERVAL) {
    data.device_id = 0;
    data.header_written = false;
    data.initial_log_done = false;
    data.is_temp_file = true;
    data.start_time = 0;
    data.last_log_time = 0;
    data.current_log_filename[0] = '\0';
}

bool BatteryLogger::activate(bool retry) {
    // Call parent class activate first
    if (!ProcessWorker<BatteryLogEntry>::activate(retry)) {
        return false;
    }

    M5_LOGI("Activating BatteryLogger");

    // Store activation time
    data.start_time = millis();
    data.last_log_time = 0;
    data.header_written = false;
    data.initial_log_done = false;
    data.is_temp_file = true;

    // Don't activate until SD card is ready and system is initialized
    if (!SDInterface::i().can_write_logs()) {
        M5_LOGV("SD card not ready for writing, delaying BatteryLogger activation");
        return false;
    }


    
    // Set up the temporary log file path
    strncpy(data.current_log_filename, BATT_LOG_TEMP_FILENAME, sizeof(data.current_log_filename) - 1);
    data.current_log_filename[sizeof(data.current_log_filename) - 1] = '\0';

    // Prepare log file (create directory and temp file, deleting any previous one)
    if (!SDInterface::i().setup_log(BATT_LOG_DIR, data.current_log_filename, true)) {
        M5_LOGE("Failed to create battery log directory or log file");
        return false;
    }
    strncpy(data.current_log_filename, BATT_LOG_TEMP_FILENAME, sizeof(data.current_log_filename) - 1);
    data.current_log_filename[sizeof(data.current_log_filename) - 1] = '\0';
    

    
    M5_LOGI("Created new battery log file: %s", data.current_log_filename);
    data.header_written = false;

    M5_LOGI("BatteryLogger activated. Log file: %s", data.current_log_filename);
    return true;
}

void BatteryLogger::deactivate() {
    M5_LOGI("Deactivating BatteryLogger");
    
    // If we haven't logged enough data, clean up the log file
    if (data.initial_log_done && data.last_log_time - data.start_time < BATTERY_LOG_INTERVAL * 2) {
        M5_LOGI("Removing incomplete battery log file");
        SDInterface::i().delete_log(data.current_log_filename);
    }
    
    // Call parent class deactivate
    ProcessWorker<BatteryLogEntry>::deactivate();
}



    


int8_t BatteryLogger::produce_data(const WorkerMap& workers) {
    // Check if we should log (every BATTERY_LOG_INTERVAL ms)
        uint32_t current_millis = millis();
    // Skip until next 10-minute interval after initial log
    if (data.initial_log_done && (current_millis - data.last_log_time < BATTERY_LOG_INTERVAL)) {
        return e_worker_idle;
    }

    // Ensure device ID is initialized
    if (data.device_id == 0) {
        auto* storage = workers.worker<LocalStorage>(k_worker_local_storage);
        if (storage) {
            data.device_id = storage->get_device_id();
        }
    }
    
    // Write header (once we have device id and mode)
    if (!data.header_written && data.device_id != 0) {
        const auto* storage = workers.worker<LocalStorage>(k_worker_local_storage);
        const char* mode_str = "unknown";
        if (storage) {
            switch (storage->get_last_mode()) {
                case LocalStorage::e_operational_mode_drive: mode_str = "drive"; break;
                case LocalStorage::e_operational_mode_survey: mode_str = "survey"; break;
                case LocalStorage::e_operational_mode_fixed: mode_str = "fixed"; break;
                case LocalStorage::e_operational_mode_satellite: mode_str = "cosmic"; break;
                case LocalStorage::e_operational_mode_flight: mode_str = "flight"; break;
            }
        }
        char comment[64];
        sprintf(comment, "# device_id=%u", data.device_id);
        SDInterface::i().log_println(data.current_log_filename, comment);
        sprintf(comment, "# mode=%s", mode_str);
        SDInterface::i().log_println(data.current_log_filename, comment);
        SDInterface::i().log_println(data.current_log_filename, BATT_LOG_HEADER);
        data.header_written = true;
    }

    // Check if SD card is ready
    if (!SDInterface::i().can_write_logs()) {
        M5_LOGV("SD card not ready for writing, skipping battery log");
        return e_worker_idle;
    }
    
    // Get required workers
    auto* battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
    if (!battery) {
        M5_LOGE("Failed to get BatteryIndicator worker");
        return e_worker_error;
    }

    // Get battery data
    const auto& battery_data = battery->get_data();
    float battery_voltage = battery_data.voltage;
    float battery_current = battery_data.current_mA;

    // (Optional) log file rename based on RTC could go here
    // Disabled for now until fully implemented


        if (data.start_time == 0) {
        data.start_time = current_millis;
    }

    int battery_percent = battery_data.percentage;
    unsigned long uptime_s = (current_millis - data.start_time) / 1000;
    char row[120];
    sprintf(row, "%u,%lu,%.3f,%d,%.1f", data.device_id, uptime_s, battery_voltage, battery_percent, battery_current);
    SDInterface::i().log_println(data.current_log_filename, row);

    // Update state
    if (!data.initial_log_done) {
        data.initial_log_done = true;
    }
    data.last_log_time = current_millis;

    M5_LOGD("Logged battery data: %s", row);

    return e_worker_data_read;
}
