#include "battery_logger.h"
#include "identifiers.h"
#include "utils/sd_wrapper.h"
#include "workers/battery_indicator.h"
#include "workers/local_storage.h"
#include "workers/rtc_connector.h"
#include <string.h> // Required for strncpy
#include <stdlib.h> // Required for abs

BatteryLogger::BatteryLogger(LocalStorage& config) : Handler(), _config(config) {
    _log_filename[0] = '\0'; // Initialize filename as empty
}

bool BatteryLogger::activate(bool retry) {
    _last_battery_percentage = -1; // Reset percentage on activate
    // The filename will be set by the Controller when the SD card is ready
    M5_LOGD("BatteryLogger: Activated, waiting for filename.");
    return true; // Activation is successful if we can reset internal state
}

void BatteryLogger::deactivate() {
    _last_battery_percentage = -1; // Reset percentage on deactivate
    M5_LOGD("BatteryLogger: Deactivated.");
}

void BatteryLogger::set_log_filename(const char* filename) {
    strncpy(_log_filename, filename, sizeof(_log_filename) - 1);
    _log_filename[sizeof(_log_filename) - 1] = '\0'; // Ensure null termination
    M5_LOGD("BatteryLogger: Log filename set to %s.", _log_filename);
}

void BatteryLogger::log_battery_level_before_shutdown(const worker_map_t& workers) {
    // Get the battery and RTC data from the workers
    const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
    const auto& rtc_data = workers.worker<DateTimeProvider>(k_worker_rtc_connector)->get_data();
    
    // Check if battery data and time are valid
    if (battery_data.percentage < 0 || !rtc_data.valid) {
        M5_LOGE("BatteryLogger: Failed to log battery level before shutdown - invalid data");
        return;
    }

    // Format log line: Date,Time,BatteryLevel,SHUTDOWN
    char log_line[100];
    sprintf(log_line, "%04d-%02d-%02d,%02d:%02d:%02d,%d,SHUTDOWN",
            rtc_data.year, rtc_data.month, rtc_data.day,
            rtc_data.hour, rtc_data.minute, rtc_data.second,
            battery_data.percentage);
    
    // Write to log file with special marker
    if (SDInterface::i().log_println(_log_filename, log_line)) {
        M5_LOGI("BatteryLogger: Logged battery level %d%% at shutdown", battery_data.percentage);
    } else {
        M5_LOGE("BatteryLogger: Failed to write shutdown log line");
    }
}

int8_t BatteryLogger::handle_produced_work(const worker_map_t& workers) {
    // Check if log file is ready (filename has been set by the Controller)
    if (_log_filename[0] == '\0') {
        return e_handler_idle; // Not ready to log yet
    }

    // Use the workers map to get the battery and RTC data
    const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
    const auto& rtc_data = workers.worker<DateTimeProvider>(k_worker_rtc_connector)->get_data();
    
    // Check if battery data and time are valid
    if (battery_data.percentage < 0 || !rtc_data.valid) {
        return e_handler_idle; // No valid data yet
    }

    // Check for 5% change in battery level
    if (_last_battery_percentage == -1 || abs(battery_data.percentage - _last_battery_percentage) >= 5) {
        // Format log line: Date,Time,BatteryLevel
        char log_line[100];
        sprintf(log_line, "%04d-%02d-%02d,%02d:%02d:%02d,%d",
                rtc_data.year, rtc_data.month, rtc_data.day,
                rtc_data.hour, rtc_data.minute, rtc_data.second,
                battery_data.percentage);
        
        // Write to log file
        if (SDInterface::i().log_println(_log_filename, log_line)) {
            _last_battery_percentage = battery_data.percentage;
            M5_LOGD("BatteryLogger: Logged battery level %d%% at %02d:%02d:%02d", battery_data.percentage, rtc_data.hour, rtc_data.minute, rtc_data.second);
            return e_handler_data_handled;
        } else {
            M5_LOGE("BatteryLogger: Failed to write log line.");
            // Deactivate if writing fails, potentially due to SD card issue
            deactivate(); // This will clear the filename, stopping further writes
            return e_handler_error;
        }
    }

    return e_handler_idle; // No significant change in battery level
}

// Removed create_log_file function as its logic will be moved to the Controller. 