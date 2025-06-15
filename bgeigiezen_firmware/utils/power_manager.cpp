#include "power_manager.h"
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
#include <M5Unified.hpp>

// Initialize static members
bool PowerManager::_low_power_mode = false;
uint32_t PowerManager::_original_cpu_freq = 240; // Default to 240MHz
uint32_t PowerManager::_original_i2c_freq = 100000; // Default to 100kHz
int PowerManager::_last_power_reading = 0;
uint32_t PowerManager::_last_cpu_freq = 0;
uint32_t PowerManager::_last_i2c_freq = 0;

void PowerManager::begin() {
    _low_power_mode = false;
    _original_cpu_freq = getCpuFrequency();
    _original_i2c_freq = getCurrentI2cClock();
    POWER_LOG_I("Power Manager initialized");
}

void PowerManager::enterLowPowerMode() {
    if (_low_power_mode) {
        POWER_LOG_W("Already in low power mode");
        return;
    }

    POWER_LOG_I("Entering low power mode");
    
    // Disable wireless
    disableWireless();
    
    // Set CPU frequency to 40MHz
    if (!setCpuFrequency(40)) {
        POWER_LOG_E("Failed to set CPU frequency to 40MHz");
    }
    
    // Set I2C clock to 50kHz
    setI2cClock(50000);
    
    _low_power_mode = true;
    logPowerState();
}

void PowerManager::exitLowPowerMode() {
    if (!_low_power_mode) {
        POWER_LOG_W("Not in low power mode");
        return;
    }

    POWER_LOG_I("Exiting low power mode");
    
    // Restore original settings
    restoreNormalSettings();
    
    _low_power_mode = false;
    logPowerState();
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
        return true;
    }
    
    // Validate frequency for ESP32-S3
    bool valid_freq = false;
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        // ESP32-S3 supported frequencies
        if (freq_mhz == 80 || freq_mhz == 160 || freq_mhz == 240) {
            valid_freq = true;
        }
    #else
        // Original ESP32 frequencies
        if (freq_mhz == 80 || freq_mhz == 160 || freq_mhz == 240) {
            valid_freq = true;
        }
    #endif
    
    if (!valid_freq) {
        ESP_LOGW("PowerMgr", "Unsupported CPU frequency: %u MHz, using 80MHz", freq_mhz);
        freq_mhz = 80;
    }
    
    ESP_LOGI("PowerMgr", "Changing CPU frequency from %u MHz to %u MHz", 
            getCpuFrequencyMhz(), freq_mhz);
    
    // Set CPU frequency using Arduino-ESP32 API
    bool success = setCpuFrequencyMhz(freq_mhz);
    
    if (!success) {
        ESP_LOGE("PowerMgr", "Failed to set CPU frequency to %u MHz", freq_mhz);
        return false;
    }
    
    // Verify
    if (getCpuFrequencyMhz() != freq_mhz) {
        ESP_LOGE("PowerMgr", "Failed to verify CPU frequency set to %u MHz", freq_mhz);
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
    
    if (!_low_power_mode) {
        // Only update original frequency if not in low power mode
        _original_i2c_freq = Wire.getClock();
    }
    
    ESP_LOGI("PowerMgr", "Setting I2C clock to %u Hz (was %u Hz)", freq_hz, Wire.getClock());
    Wire.setClock(freq_hz);
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
    _last_i2c_freq = Wire.getClock();
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
