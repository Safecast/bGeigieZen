#include "battery_logger.h"
#include "identifiers.h"
#include "workers/rtc_connector.h"
#include "workers/battery_indicator.h"
#include "workers/local_storage.h"
// #include "controller.h" // No longer needed for DeviceState::Mode here
#include <SD.h>       // For SD card operations
#include <cmath>      // For std::abs
#include <cstdio>     // For snprintf
#include <cstring>    // For strncpy
#include <time.h>       // For mktime for epoch conversion

// Helper function to convert OperationalMode to string
const char* get_operational_mode_string(LocalStorage::OperationalMode mode) {
    switch (mode) {
        case LocalStorage::e_operational_mode_drive:
            return "DRIVE_MODE";
        case LocalStorage::e_operational_mode_survey:
            return "SURVEY_MODE";
        case LocalStorage::e_operational_mode_fixed:
            return "FIXED_MODE";
        case LocalStorage::e_operational_mode_satellite:
            return "SATELLITE_MODE";
        case LocalStorage::e_operational_mode_flight:
            return "FLIGHT_MODE";
        default:
            return "UNKNOWN_MODE";
    }
}

#define BATTERY_LOG_DIRECTORY "/battery_logs"
// #define BATTERY_LOG_FILE_PREFIX "battery_log_" // No longer used directly for initial file
// #define BATTERY_LOG_FILE_SUFFIX ".csv" // No longer used directly for initial file
#define BATT_TEMP_LOG_NAME_F "%s/latest_batt.csv"
#define BATT_DATED_LOG_NAME_F "%s/batt_%04d-%02d-%02d_%02d%02d.csv"
#define BATTERY_LOG_INTERVAL 5000  // 5 seconds between checks
#define BATTERY_LEVEL_THRESHOLD 5   // Log when battery level changes by 5%

BatteryLogger::BatteryLogger()
    : ProcessWorker<BatteryLogEntry>(BATTERY_LOG_INTERVAL) {
    data.last_battery_level = 0;
    data.last_log_time = 0;
    data.device_id = 0;
    data.header_written = false;
    data.initial_log_done = false;
    data.is_temp_file = true; // Ensure it's true on construction
}

bool BatteryLogger::activate(bool retry) {
    // Ensure the directory exists
    if (!SD.exists(BATTERY_LOG_DIRECTORY)) {
        if (!SD.mkdir(BATTERY_LOG_DIRECTORY)) {
            M5_LOGE("Failed to create battery log directory: %s", BATTERY_LOG_DIRECTORY);
            return false; // Critical if we can't create the directory
        }
    }

    // Set up the temporary log file path
    snprintf(data.current_log_filename, sizeof(data.current_log_filename),
             BATT_TEMP_LOG_NAME_F, BATTERY_LOG_DIRECTORY);

    // Delete existing temporary log file to start fresh, if it exists
    if (SD.exists(data.current_log_filename)) {
        SD.remove(data.current_log_filename);
    }

    M5_LOGI("BatteryLogger activated. Initial log file: %s", data.current_log_filename);
    data.header_written = false;    // Header will need to be written to the new temp file
    data.initial_log_done = false;  // Reset for the new session
    data.is_temp_file = true;       // Mark that we are using a temporary file

    return true;
}

int8_t BatteryLogger::produce_data(const worker_map_t& workers) {
    // M5_LOGI("BatteryLogger::produce_data(workers) ENTRYPOINT REACHED"); // Disabled for less verbose logging
    uint32_t current_millis = millis();
    const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    const auto& rtc_worker = workers.worker<DateTimeProvider>(k_worker_rtc_connector); // Corrected type
    // const auto& controller_worker = workers.worker<Controller>(k_worker_device_state); // No longer needed

    if (!battery || !settings /*|| !controller_worker*/) { // controller_worker removed from check
        M5_LOGE("Required worker not available for BatteryLogger");
        return e_worker_error;
    }

    // Use active() method, not is_active()
    if (!battery->active()) { 
        M5_LOGI("Battery indicator not active, BatteryLogger idle.");
        return e_worker_idle;
    }

    // uint32_t current_millis = millis(); // Removed: Duplicate of declaration at line 76
    // Get battery level and voltage from data struct: battery->get_data().percentage, battery->get_data().voltage
    int battery_level = battery->get_data().percentage;
    float battery_voltage = battery->get_data().voltage;
    // DeviceState::Mode current_mode = controller_worker->get_data().mode; // No longer needed

    bool should_log = false;

    // Log if it's the initial log or if battery level changed by threshold
    if (!data.initial_log_done) {
        M5_LOGI("Performing initial battery log.");
        should_log = true;
    }
    else if (std::abs(battery_level - (int)data.last_battery_level) >= BATTERY_LEVEL_THRESHOLD) {
        M5_LOGI("Battery level change detected. Old: %d, New: %d", data.last_battery_level, battery_level);
        should_log = true;
    }
    // Add special case for 0% battery level - log every 5 minutes
    else if (battery_level == 0 && (current_millis - data.last_log_time >= 300000)) { // 300000ms = 5 minutes
        M5_LOGI("Battery at 0%, logging 5-minute interval entry.");
        should_log = true;
    }

    // Note: Logging due to mode change or simple interval pass is removed as per new requirement.

    if (should_log) {
        uint32_t timestamp_s;
        char dt_buffer[30] = "N/A"; // For YYYY-MM-DDTHH:MM:SSZ string or N/A
        bool rtc_is_reliable_for_entry = rtc_worker && rtc_worker->active() && rtc_worker->get_data().valid;

        // Determine timestamp for the log entry itself
        if (rtc_is_reliable_for_entry) {
            const auto& rtc_data = rtc_worker->get_data();
            struct tm t;
            t.tm_year = rtc_data.year - 1900;
            t.tm_mon = rtc_data.month - 1;
            t.tm_mday = rtc_data.day;
            t.tm_hour = rtc_data.hour;
            t.tm_min = rtc_data.minute;
            t.tm_sec = rtc_data.second;
            t.tm_isdst = -1; // Not using DST info
            timestamp_s = mktime(&t);
            snprintf(dt_buffer, sizeof(dt_buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                     rtc_data.year, rtc_data.month, rtc_data.day,
                     rtc_data.hour, rtc_data.minute, rtc_data.second);
            M5_LOGI("Using RTC for log entry timestamp: %s (%u)", dt_buffer, timestamp_s);
        } else {
            timestamp_s = current_millis / 1000;
            M5_LOGI("RTC not available for log entry timestamp, using millis: %u", timestamp_s);
        }

        // Handle log filename: if temp file is in use and RTC is now available, rename it.
        if (data.is_temp_file && rtc_is_reliable_for_entry) {
            const auto& rtc_data = rtc_worker->get_data(); // Safe due to rtc_is_reliable_for_entry check
            char final_filename[128];
            snprintf(final_filename, sizeof(final_filename),
                     BATT_DATED_LOG_NAME_F, BATTERY_LOG_DIRECTORY,
                     rtc_data.year, rtc_data.month, rtc_data.day,
                     rtc_data.hour, rtc_data.minute);

            M5_LOGI("Attempting to rename temp log '%s' to final log '%s'", data.current_log_filename, final_filename);
            if (SD.exists(data.current_log_filename)) {
                if (SD.rename(data.current_log_filename, final_filename)) {
                    M5_LOGI("Successfully renamed log file to: %s", final_filename);
                    strncpy(data.current_log_filename, final_filename, sizeof(data.current_log_filename) - 1);
                    data.current_log_filename[sizeof(data.current_log_filename) - 1] = '\0';
                    data.is_temp_file = false;
                    // The header was already written to the temp file, so it's now in the renamed file.
                    // data.header_written status is preserved.
                } else {
                    M5_LOGE("Failed to rename battery log file from '%s' to '%s'. Will continue logging to temp file.", data.current_log_filename, final_filename);
                    // If rename fails, we continue using the temp file. data.is_temp_file remains true.
                }
            } else {
                M5_LOGW("Temporary log file '%s' did not exist for renaming. Will attempt to use/create final name '%s' directly.", data.current_log_filename, final_filename);
                strncpy(data.current_log_filename, final_filename, sizeof(data.current_log_filename) - 1);
                data.current_log_filename[sizeof(data.current_log_filename) - 1] = '\0';
                data.is_temp_file = false; // Now targeting the final name
                data.header_written = false; // New file (or assumed new), so header needs to be written
            }
        }
        // At this point, data.current_log_filename is set to either the temp file or the final dated file.


        // Use SD object for filesystem operations
        if (!SD.exists(BATTERY_LOG_DIRECTORY)) {
            M5_LOGI("Battery log directory %s does not exist, creating.", BATTERY_LOG_DIRECTORY);
            if (!SD.mkdir(BATTERY_LOG_DIRECTORY)) {
                M5_LOGE("Failed to create battery log directory: %s", BATTERY_LOG_DIRECTORY);
                // data.initial_log_done is not reset here. If it was false, it remains false for the next attempt.
                // If it was true, it remains true.
                return e_worker_error;
            }
        }

        M5_LOGI("Attempting to open or create log file: %s", data.current_log_filename);
        File file = SD.open(data.current_log_filename, FILE_APPEND);
        if (!file) {
            M5_LOGE("Failed to open battery log file: %s", data.current_log_filename);
            // data.initial_log_done is not reset here. If it was false, it remains false for the next attempt.
            // If it was true, it remains true.
            return e_worker_error;
        }

        M5_LOGI("Successfully opened log file: %s", data.current_log_filename);

        if (file.size() == 0 || !data.header_written) {
            M5_LOGI("Writing CSV header to new log file: %s", data.current_log_filename);
            // Get operational mode for the header comment
            LocalStorage* settings = workers.worker<LocalStorage>(k_worker_local_storage);
            const char* mode_str = "UNKNOWN_MODE";
            if (settings && settings->active()) {
                mode_str = get_operational_mode_string(settings->get_last_mode());
            }
            file.printf("# Operational Mode: %s\n", mode_str);
            file.println("device_id,timestamp_utc,datetime_utc,battery_level_percent,battery_voltage");
            data.header_written = true;
        }

        // Ensure device_id is fetched if not already cached
        if (data.device_id == 0) {
            data.device_id = settings->get_device_id();
             if (data.device_id == 0) { // Still 0 after trying to fetch?
                M5_LOGW("Device ID is 0. Check LocalStorage settings.");
            }
        }

        // Prepare final log entry string using dt_buffer
        char final_log_buffer[256]; // Use a distinct name to avoid confusion if log_buffer was used elsewhere
        snprintf(final_log_buffer, sizeof(final_log_buffer), "%u,%u,%s,%d,%.3f",
                 data.device_id,
                 timestamp_s,      // epoch timestamp
                 dt_buffer,        // YYYY-MM-DDTHH:MM:SSZ or "N/A"
                 battery_level,
                 battery_voltage);
        
        M5_LOGI("Writing log entry: %s", final_log_buffer);
        file.println(final_log_buffer);
        file.close();

        data.last_battery_level = battery_level;
        // data.last_mode update removed as field is gone from BatteryLogEntry
        data.last_log_time = current_millis;
        if (!data.initial_log_done) { // Set initial_log_done only after the first successful log
            data.initial_log_done = true;
        }
        
        M5_LOGI("Battery log written successfully.");
        return e_worker_data_read;
    }

    return e_worker_idle;
}
