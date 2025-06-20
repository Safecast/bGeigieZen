#ifndef BGEIGIEZEN_BATTERY_LOGGER_H_
#define BGEIGIEZEN_BATTERY_LOGGER_H_
#define BATT_LOG_DIRNAME "BATT"
#define BATTERY_LOG_INTERVAL 600000 // 10 minutes between checks

#include <stdint.h>
#include <Worker.hpp>      // Base class from SensorReporter
#include <Handler.hpp>     // For ProcessWorker from SensorReporter
#include "workers/battery_indicator.h"
#include "workers/rtc_connector.h" // For DateTimeProvider
#include "workers/local_storage.h"
#include "identifiers.h"         // For k_worker_* constants

#include "utils/sd_wrapper.h"


// Forward declaration for WorkerMap
class WorkerMap;

// Define BatteryLogEntry here, after controller.h to ensure DeviceState is known
struct BatteryLogEntry {
    uint32_t device_id;
    float    battery_voltage;    // Battery voltage in volts
    bool     header_written;     // Tracks if the CSV header has been written for the current file
    bool     initial_log_done;   // Tracks if the very first log entry has been made
    char     current_log_filename[128]; // Stores the name of the current daily log file
    bool     is_temp_file;       // Tracks if the current log filename is temporary
    uint32_t start_time;       // Milliseconds from millis() when device powered up
    uint32_t last_log_time;    // Milliseconds from millis() when last log entry was made

    BatteryLogEntry() :
        device_id(0),
        battery_voltage(0.0f),
        header_written(false),
        initial_log_done(false),
        is_temp_file(true), // Initialize as temporary
        start_time(0),       // Initialize start_time
        last_log_time(0) {   // Initialize last_log_time
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

    // Override methods from ProcessWorker
    bool activate(bool retry = false) override;


protected:
    void deactivate() override;
    int8_t produce_data(const WorkerMap& workers) override;
};

#endif //BGEIGIEZEN_BATTERY_LOGGER_H_
