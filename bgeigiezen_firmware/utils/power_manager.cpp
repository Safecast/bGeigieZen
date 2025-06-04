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
#include "handlers/battery_logger.h"
#include "controller.h"

#include "workers/local_storage.h"
#include "rtc_wdt_wrapper.h"

// Initialize static members


void PowerManager::setController(Controller* controller) {
    _controller = controller;
}

void PowerManager::begin() {
    _low_power_mode = false;
    _original_cpu_freq = getCpuFrequencyMhz();
    _original_i2c_freq = Wire.getClock();
    rtc_wdt_wrapper_init();
}

void PowerManager::enterLowPowerMode() {
    PowerManager& instance = PowerManager::instance();
    if (instance._controller) {
        instance._controller->data.mode = DeviceState::e_mode_simple;
        M5.Lcd.setBrightness(0);
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
        esp_wifi_set_max_tx_power(WIFI_PHY_MODE_LR);
        esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
        Wire.setClock(100000);
        setCpuFrequencyMhz(80);
        
        // Log battery level before shutdown
        instance.logBatteryLevelBeforeShutdown(*instance._controller);
        
        instance._low_power_mode = true;
    }
    setI2cClock(50000);  // 50kHz is sufficient for most sensors
    
    // 3. Disable WiFi and Bluetooth
    disableWireless();
    
    // 4. Keep battery logger active during low power mode
    if (_battery_logger && _settings && _controller) {
        logBatteryLevelBeforeShutdown(*instance._controller);
    }
    
    // 5. Reduce logging to minimum
}

void PowerManager::exitLowPowerMode() {
    if (!_low_power_mode) return;  // Not in low power mode
    
    M5_LOGI("Exiting low power mode");
    
    // 1. Restore CPU frequency
    if (_original_cpu_freq > 0) {
        setCpuFrequency(_original_cpu_freq);
    }
    
    // 2. Restore I2C clock speed
    if (_original_i2c_freq > 0) {
        setI2cClock(_original_i2c_freq);
    }
    
    // 3. Deactivate battery logger
    if (_battery_logger) {
        _battery_logger->deactivate();
    }
    
    // Restore normal logging
    esp_log_level_set("*", ESP_LOG_INFO);
    
    // Note: WiFi/BT will be re-enabled when needed by the specific mode
    
    _low_power_mode = false;
    M5_LOGI("Normal power mode restored");
}

void PowerManager::logBatteryLevelBeforeShutdown(Controller& controller) {
    if (!_battery_logger || !_settings) {
        return;
    }
    _battery_logger->log_battery_level_before_shutdown(controller.workers);
}

void PowerManager::setBatteryLogger(BatteryLogger* logger) {
    PowerManager& instance = PowerManager::instance();
    instance._battery_logger = logger;
}

void PowerManager::setSettings(LocalStorage* settings) {
    PowerManager& instance = PowerManager::instance();
    instance._settings = settings;
}

bool PowerManager::setCpuFrequency(uint32_t freq_mhz) {
    PowerManager& instance = PowerManager::instance();
    if (freq_mhz == 0) {
        freq_mhz = instance._original_cpu_freq;  // Restore original frequency if 0 is passed
    }
    
    // Store the original frequency if not already stored
    if (instance._original_cpu_freq == 0) {
        instance._original_cpu_freq = getCpuFrequencyMhz();
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
        M5_LOGW("Unsupported CPU frequency: %u MHz, using 80MHz", freq_mhz);
        freq_mhz = 80;
    }
    
    M5_LOGI("Changing CPU frequency from %u MHz to %u MHz", 
            getCpuFrequencyMhz(), freq_mhz);
    
    // Set CPU frequency using Arduino-ESP32 API
    bool success = setCpuFrequencyMhz(freq_mhz);
    
    if (!success) {
        M5_LOGE("Failed to set CPU frequency to %u MHz", freq_mhz);
        return false;
    }
    
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
    M5_LOGI("Disabling wireless modules");
    
    // Disable WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    
    // Disable Bluetooth
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
}

void PowerManager::enableWireless() {
    M5_LOGI("Wireless modules will be re-enabled when needed by specific modes");
    // Note: WiFi and BT are enabled on-demand by the specific features that need them
}

void PowerManager::setI2cClock(uint32_t freq_hz) {
    PowerManager& instance = PowerManager::instance();
    if (freq_hz < 10000 || freq_hz > 1000000) {
        M5_LOGW("I2C frequency %u Hz is outside recommended range (10kHz - 1MHz)", freq_hz);
    }
    
    if (!instance._low_power_mode) {
        instance._original_i2c_freq = Wire.getClock();
    }
    M5_LOGI("Setting I2C clock to %u Hz (was %u Hz)", freq_hz, Wire.getClock());
    Wire.setClock(freq_hz);
}

// Brownout detector control removed for ESP32-S3 compatibility
// The brownout detector is a safety feature that prevents damage
// from low voltage conditions. It's better to keep it enabled.
