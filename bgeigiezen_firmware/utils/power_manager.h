#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <driver/rtc_io.h>
#include <soc/rtc.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/sens_reg.h>
#include "rtc_wdt_wrapper.h"

// Define logging macros if not already defined
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
public:
    /**
     * @brief Initialize power management
     */
    static void begin();

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
    static void enterLowPowerMode();

    /**
     * @brief Exit low power mode
     * 
     * This function should be called when leaving Cosmic mode to restore normal operation.
     */
    static void exitLowPowerMode();

    /**
     * @brief Set the CPU frequency
     * 
     * @param freq_mhz CPU frequency in MHz (80, 160, 240)
     * @return true if successful, false otherwise
     */
    static bool setCpuFrequency(uint32_t freq_mhz);

    /**
     * @brief Get the current CPU frequency
     * 
     * @return uint32_t Current CPU frequency in MHz
     */
    static uint32_t getCpuFrequency();

    /**
     * @brief Disable WiFi and Bluetooth radios
     */
    static void disableWireless();

    /**
     * @brief Re-enable WiFi and Bluetooth radios
     */
    static void enableWireless();

    /**
     * @brief Set the I2C clock speed
     * 
     * @param freq_hz I2C clock frequency in Hz (default: 100000)
     */
    static void setI2cClock(uint32_t freq_hz = 100000);

    /**
     * @brief Restore all settings to their original values
     */
    static void restoreNormalSettings();

private:
    static bool _low_power_mode;             ///< Whether we're in low power mode
    static uint32_t _original_cpu_freq;      ///< Original CPU frequency
    static uint32_t _original_i2c_freq;      ///< Original I2C frequency
};

#endif // POWER_MANAGER_H
