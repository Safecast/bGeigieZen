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
// #define BATTERY_LOG_INTERVAL 5000  // 5 seconds between checks
// #define BATTERY_LEVEL_THRESHOLD 5   // Log when battery level changes by 5%

BatteryLogger::BatteryLogger()
    : ProcessWorker<BatteryLogEntry>(BATTERY_LOG_INTERVAL) {
    data.device_id = 0;
    data.header_written = false;
    data.initial_log_done = false;
    data.is_temp_file = true; // Ensure it's true on construction
    data.start_time = 0;       // Initialize start_time
    data.last_log_time = 0;    // Initialize last_log_time
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
    data.start_time = millis();     // Record the power-up time

    return true;
}

int8_t BatteryLogger::produce_data(const worker_map_t& workers) {
    // M5_LOGI("BatteryLogger::produce_data(workers) ENTRYPOINT REACHED"); // Disabled for less verbose logging
    uint32_t current_millis = millis();
    const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    // Removed: const auto& rtc_worker = workers.worker<DateTimeProvider>(k_worker_rtc_connector);

    if (!battery || !settings) { // controller_worker removed from check
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
    // Removed: int battery_level = battery->get_data().percentage;
    float battery_voltage = battery->get_data().voltage;
    // DeviceState::Mode current_mode = controller_worker->get_data().mode; // No longer needed

    bool should_log = false;

    // Log every 10 minutes (BATTERY_LOG_INTERVAL)
    if (current_millis - data.last_log_time >= BATTERY_LOG_INTERVAL) {
        M5_LOGI("Battery logging interval reached. Logging battery data.");
        should_log = true;
    }

    if (should_log) {
        // uint32_t timestamp_s;
        // char dt_buffer[30] = "N/A"; // For YYYY-MM-DDTHH:MM:SSZ string or N/A
        // bool rtc_is_reliable_for_entry = rtc_worker && rtc_worker->active() && rtc_worker->get_data().valid;

        // Removed all RTC and filename renaming logic. Always use temporary file.

        // At this point, data.current_log_filename is set to the temp file.

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
            file.println("device_id,time_since_powerup_ms,battery_voltage"); // Simplified header
            data.header_written = true;
        }

        // Ensure device_id is fetched if not already cached
        if (data.device_id == 0) {
            data.device_id = settings->get_device_id();
             if (data.device_id == 0) { // Still 0 after trying to fetch?
                M5_LOGW("Device ID is 0. Check LocalStorage settings.");
            }
        }

        // Prepare final log entry string
        char final_log_buffer[256];
        snprintf(final_log_buffer, sizeof(final_log_buffer), "%u,%u,%.3f",
                 data.device_id,
                 current_millis - data.start_time, // Time since powerup
                 battery_voltage);
        
        M5_LOGI("Writing log entry: %s", final_log_buffer);
        file.println(final_log_buffer);
        file.close();

        // Removed: data.last_battery_level = battery_level;
        data.last_log_time = current_millis;
        if (!data.initial_log_done) { // Set initial_log_done only after the first successful log
            data.initial_log_done = true;
        }
        
        M5_LOGI("Battery log written successfully.");
        return e_worker_data_read;
    }

    return e_worker_idle;
}
