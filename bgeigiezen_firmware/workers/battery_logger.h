#ifndef BGEIGIEZEN_BATTERY_LOGGER_H_
#define BGEIGIEZEN_BATTERY_LOGGER_H_
#define BATT_LOG_DIRNAME "BATT"

#include <stdint.h>
#include <Worker.hpp>      // Base class from SensorReporter
#include <Handler.hpp>     // For ProcessWorker from SensorReporter
#include "workers/battery_indicator.h"
#include "workers/rtc_connector.h" // For DateTimeProvider
#include "workers/local_storage.h"
#include "identifiers.h"         // For k_worker_* constants

#include "utils/sd_wrapper.h"
#include "controller.h"

// Define BatteryLogEntry here, after controller.h to ensure DeviceState is known
struct BatteryLogEntry {
    uint32_t last_battery_level; // Percentage
    uint32_t last_log_time;      // Milliseconds from millis()
    uint32_t device_id;
    bool     header_written;     // Tracks if the CSV header has been written for the current file
    bool     initial_log_done;   // Tracks if the very first log entry has been made
    char     current_log_filename[128]; // Stores the name of the current daily log file
    bool     is_temp_file;       // Tracks if the current log filename is temporary

    BatteryLogEntry() :
        last_battery_level(0),
        last_log_time(0),
        device_id(0),
        header_written(false),
        initial_log_done(false),
        is_temp_file(true) { // Initialize as temporary
            current_log_filename[0] = '\0'; // Initialize to empty string
        }
};
/**
 * Battery logger worker that logs battery level changes to SD card
 */
// BatteryLoggerData struct removed as BatteryLogEntry (defined above) is used by ProcessWorker
class BatteryLogger : public ProcessWorker<BatteryLogEntry> {
public:
    explicit BatteryLogger();
    virtual ~BatteryLogger() = default;

    bool activate(bool retry) override;
    int8_t produce_data(const worker_map_t& workers) override;

private:

};

#endif //BGEIGIEZEN_BATTERY_LOGGER_H_
