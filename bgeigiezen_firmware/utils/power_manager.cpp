#include "power_manager.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <soc/rtc.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/sens_reg.h>
#include <esp32-hal-cpu.h>
#include <esp_task_wdt.h>
#include <M5Unified.hpp>
#include <WiFi.h>

// Initialize static members
bool PowerManager::_low_power_mode = false;
uint32_t PowerManager::_original_cpu_freq = 240; // Default to 240MHz
uint32_t PowerManager::_original_i2c_freq = 100000; // Default to 100kHz (logical)
int PowerManager::_last_power_reading = 0;
uint32_t PowerManager::_last_cpu_freq = 0;
uint32_t PowerManager::_last_i2c_freq = 100000; // Track logical I2C clock

void PowerManager::begin() {
    _low_power_mode = false;
    _original_cpu_freq = getCpuFrequency();
    // Under M5Unified, do not query Wire bus directly here.
    _original_i2c_freq = 100000; // assume 100kHz default
    _last_i2c_freq = _original_i2c_freq;
    POWER_LOG_I("Power Manager initialized");
}

void PowerManager::enterLowPowerMode(bool wifi_required) {
    if (_low_power_mode) {
        M5_LOGW("Already in low power mode");
        return;
    }

    M5_LOGI("Entering low power mode (WiFi %s)", wifi_required ? "required" : "not required");
    logPowerState(); // Log initial state

    // 1. Set CPU frequency for power savings
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        // ESP32-S3 supports 80, 160, 240 MHz natively. We'll use 80MHz for stable low power.
        setCpuFrequency(80);
        logPowerState(); // Log after CPU frequency change
    #else
        // For Core2 (ESP32):
        // CPU is set to 80MHz at boot and NEVER changed
        // WiFi buffer allocation happens at 80MHz and stays stable
        // This provides ~33% power savings compared to 240MHz
        M5_LOGI("Core2: Keeping CPU at 80MHz (set at boot) for power savings and WiFi stability");
    #endif

    // 2. Reduce I2C clock speed (logical only; transfers specify freq per call)
    setI2cClock(50000);  // 50kHz logical target
    logPowerState(); // Log after I2C clock change

    // 3. Wireless modules will be managed by specific modes
    
    // 4. Reduce logging to minimum, but keep WiFi debug logging
    esp_log_level_set("*", ESP_LOG_ERROR);
    esp_log_level_set("wifi", ESP_LOG_DEBUG); // Set WiFi logging to DEBUG
    
    _low_power_mode = true;
    M5_LOGI("Low power mode activated");
    logPowerState(); // Log final state
}

void PowerManager::exitLowPowerMode() {
    if (!_low_power_mode) {
        M5_LOGW("Not in low power mode");
        return;
    }

    M5_LOGI("Exiting low power mode");

    // 1. Restore CPU frequency
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // CoreS3 can restore to any frequency
    if (_original_cpu_freq > 0) {
        setCpuFrequency(_original_cpu_freq);
    }
    #else
    // Core2: Don't change CPU frequency (always 80MHz for WiFi stability)
    M5_LOGI("Core2: Keeping CPU at 80MHz for WiFi stability");
    #endif

    // 2. Restore I2C clock speed (logical only)
    if (_original_i2c_freq > 0) {
        setI2cClock(_original_i2c_freq);
    }

    // Restore normal logging
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("wifi", ESP_LOG_INFO); // Restore WiFi logging to INFO

    // Note: WiFi will be initialized on-demand when needed by config mode or other features
    // We don't force initialization here to avoid memory allocation issues

    _low_power_mode = false;
    M5_LOGI("Normal power mode restored");
}

bool PowerManager::setCpuFrequency(uint32_t freq_mhz) {
    if (freq_mhz == 0) {
        freq_mhz = _original_cpu_freq;  // Restore original frequency if 0 is passed
    }
    
    // Store the original frequency if not already stored
    if (_original_cpu_freq == 0) {
        _original_cpu_freq = getCpuFrequencyMhz();
    }
    
    // Check if frequency is already set
    if (getCpuFrequencyMhz() == freq_mhz) {
        M5_LOGI("CPU frequency already set to %u MHz", freq_mhz);
        return true;
    }
    
    // Validate frequency for ESP32-S3
    bool valid_freq = false;
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        // ESP32-S3 supported frequencies
        if (freq_mhz == 80 || freq_mhz == 160 || freq_mhz == 240 || freq_mhz == 40) {
            valid_freq = true;
        }
    #else
        // Original ESP32 frequencies (Core2)
        if (freq_mhz == 80 || freq_mhz == 160 || freq_mhz == 240 || freq_mhz == 40) {
            valid_freq = true;
        }
    #endif
    
    if (!valid_freq) {
        #ifdef CONFIG_IDF_TARGET_ESP32S3
            M5_LOGW("Unsupported CPU frequency: %u MHz for S3, using 80MHz", freq_mhz);
            freq_mhz = 80;
        #else
            // For Core2 (ESP32), default to 160MHz if unsupported frequency is requested.
            M5_LOGW("Unsupported CPU frequency: %u MHz for ESP32 (Core2), using 160MHz", freq_mhz);
            freq_mhz = 160;
        #endif
    }
    
    M5_LOGI("Attempting to change CPU frequency from %u MHz to %u MHz", 
            getCpuFrequencyMhz(), freq_mhz);
    
    bool success = setCpuFrequencyMhz(freq_mhz); // Re-enable actual CPU frequency change
    M5_LOGI("CPU frequency change attempted. Result: %s", success ? "SUCCESS" : "FAILED");

    // Verify
    if (getCpuFrequencyMhz() != freq_mhz) {
        M5_LOGE("Failed to verify CPU frequency set to %u MHz", freq_mhz);
        return false;
    }
    
    return true;
}

uint32_t PowerManager::getCpuFrequency() {
    // Use the appropriate method based on the ESP32 variant
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        return getCpuFrequencyMhz();
    #else
        return getCpuFrequencyMhz();
    #endif
}

void PowerManager::disableWireless() {
    ESP_LOGI("PowerMgr", "Disabling wireless modules");
    
    // Disable WiFi
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    esp_wifi_deinit();
  
    // Disable Bluetooth if it was enabled
    if (btStarted()) {
        btStop();
    }
}

void PowerManager::enableWireless() {
    ESP_LOGI("PowerMgr", "Wireless modules will be re-enabled when needed by specific modes");
    // Note: WiFi and BT are enabled on-demand by the specific features that need them
}

void PowerManager::setI2cClock(uint32_t freq_hz) {
    if (freq_hz < 10000 || freq_hz > 1000000) {
        ESP_LOGW("PowerMgr", "I2C frequency %u Hz is outside recommended range (10kHz - 1MHz)", freq_hz);
    }
    // Maintain logical frequency; M5Unified calls pass desired freq per transaction.
    if (!_low_power_mode) {
        _original_i2c_freq = freq_hz;
    }
    ESP_LOGI("PowerMgr", "Logical I2C clock set to %u Hz", freq_hz);
    _last_i2c_freq = freq_hz;
}

// Brownout detector control removed for ESP32-S3 compatibility
// The brownout detector is a safety feature that prevents damage
// from low voltage conditions. It's better to keep it enabled.

int PowerManager::getCurrentConsumption() {
    // For now, return a placeholder value
    // TODO: Implement proper current measurement
    return _last_power_reading;
}

uint32_t PowerManager::getCurrentCpuFrequency() {
    _last_cpu_freq = getCpuFrequencyMhz();
    return _last_cpu_freq;
}

uint32_t PowerManager::getCurrentI2cClock() {
    // Return the last logical value; avoid querying uninitialized bus.
    return _last_i2c_freq;
}

void PowerManager::logPowerState() {
    POWER_LOG_I("Power State - CPU: %u MHz, I2C: %u Hz", 
                getCurrentCpuFrequency(), 
                getCurrentI2cClock());
}

void PowerManager::restoreNormalSettings() {
    // Restore CPU frequency
    if (_original_cpu_freq > 0) {
        setCpuFrequency(_original_cpu_freq);
    }
    
    // Restore I2C clock speed
    if (_original_i2c_freq > 0) {
        setI2cClock(_original_i2c_freq);
    }
    
    // Re-enable wireless
    enableWireless();
    
    // Restore normal logging
    esp_log_level_set("*", ESP_LOG_INFO);
    
    POWER_LOG_I("Normal settings restored");
}
