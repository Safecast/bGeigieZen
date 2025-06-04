#ifndef BGEIGIEZEN_POWER_MANAGER_H_
#define BGEIGIEZEN_POWER_MANAGER_H_

#include <Wire.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <soc/rtc.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/sens_reg.h>
#include <esp32-hal-cpu.h>
#include <esp_task_wdt.h>
#include "handlers/battery_logger.h"
#include "identifiers.h"
#include "workers/local_storage.h"
#include "controller.h"
#include <Worker.hpp>

// Forward declare Controller class
class Controller;

// Worker identifiers


// Define logging macros if not already defined
#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif

#ifndef M5_LOGI
#define M5_LOGI(format, ...) Serial.printf("[PowerMgr] " format "\r\n", ##__VA_ARGS__)
#endif

#ifndef M5_LOGE
#define M5_LOGE(format, ...) Serial.printf("[PowerMgr][ERROR] " format "\r\n", ##__VA_ARGS__)
#endif

#ifndef M5_LOGW
#define M5_LOGW(format, ...) Serial.printf("[PowerMgr][WARN] " format "\r\n", ##__VA_ARGS__)
#endif

/**
 * @brief Power management utility for optimizing power consumption
 */
class PowerManager {
private:
    bool _low_power_mode;             ///< Whether we're in low power mode
    uint32_t _original_cpu_freq;      ///< Original CPU frequency
    uint32_t _original_i2c_freq;      ///< Original I2C frequency
    BatteryLogger* _battery_logger;    ///< Battery logger instance
    LocalStorage* _settings;           ///< Settings instance
    Controller* _controller;           ///< Controller instance for worker access

public:
    static PowerManager& instance() {
        static PowerManager instance;
        return instance;
    }

    PowerManager() : _low_power_mode(false),
                     _original_cpu_freq(0),
                     _original_i2c_freq(0),
                     _battery_logger(nullptr),
                     _settings(nullptr),
                     _controller(nullptr) {}
    ~PowerManager() = default;

    PowerManager(const PowerManager&) = delete;
    PowerManager& operator=(const PowerManager&) = delete;

    /**
     * @brief Set the Controller instance
     */
    void setController(Controller* controller);

    /**
     * @brief Initialize power management
     */
    void begin();
    /**
     * @brief Enter low power mode to save power
     */
    void enterLowPowerMode();
    /**
     * @brief Exit low power mode and restore normal operation
     */
    void exitLowPowerMode();
    /**
     * @brief Log battery level before shutdown
     */
    void logBatteryLevelBeforeShutdown(Controller& controller);
    /**
     * @brief Set CPU frequency
     */
    bool setCpuFrequency(uint32_t freq_mhz);
    /**
     * @brief Get current CPU frequency
     */
    static uint32_t getCpuFrequency();
    /**
     * @brief Disable wireless modules (WiFi and Bluetooth)
     */
    static void disableWireless();
    /**
     * @brief Enable wireless modules when needed
     */
    static void enableWireless();
    /**
     * @brief Set I2C clock frequency
     */
    static void setI2cClock(uint32_t freq_hz = 100000);

    /**
     * @brief Set the battery logger instance
     */
    static void setBatteryLogger(BatteryLogger* logger);
    /**
     * @brief Set the local storage settings
     */
    static void setSettings(LocalStorage* settings);

    /**
     * @brief Enter low power mode for Cosmic mode
     * 
     * This function should be called when entering Cosmic mode to minimize power consumption.
     * It will:
     * 1. Disable WiFi and Bluetooth
     * 2. Reduce CPU frequency
     * 3. Disable brownout detector
     * 4. Reduce I2C clock speed
     * 5. Disable debug output
     */
    static void enterLowPowerModeCosmic();

    /**
     * @brief Exit low power mode for Cosmic mode
     * 
     * This function should be called when leaving Cosmic mode to restore normal operation.
     */
    static void exitLowPowerModeCosmic();

    /**
     * @brief Restore all settings to their original values
     */
    static void restoreNormalSettings();
};

#endif // POWER_MANAGER_H
