/**
 * The bGeigieZen is the next generation, modern Open Source customizable Geiger Counter. The followup to the
 * successful bGeigieNanoKit.
 *
 * Homepage: https://bgeigiezen.safecast.jp/
 * GitHub https://github.com/Safecast/bGeigieZen
 * wiki: https://github.com/Safecast/bGeigieZen/wiki
 * Slack channel: #bgeigiezen
 *
 *  Copyright (c) 2026, Safecast

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *
 * History Versions:
 *
 *
 * Contact:
 *
 */

#include <M5Unified.hpp>

#include <Arduino.h>

#include "controller.h"
#include "gfx_screen.h"
#include "handlers/api_connector.h"
#include "handlers/bluetooth_reporter.h"
#include "handlers/sd_logger.h"
#include "handlers/debug_logger.h"
#include "identifiers.h"
#include "workers/battery_led_indicator.h"
#include "workers/configuration_server.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/local_storage.h"
#include "workers/rtc_connector.h"
#include "workers/battery_led_indicator.h"
#include "workers/battery_indicator.h"
// Wire.h included via M5Unified.hpp
#include "workers/zen_button.h"
#include "workers/sound_manager.h"
#include "workers/shake_detector.h"
 
#include <nvs_flash.h> // Include for NVS flash initialization

TeenyUbloxConnect gnss;
LocalStorage settings;
Controller controller(settings, gnss);

// Workers
ZenButton zen_A(M5.BtnA);
ZenButton zen_B(M5.BtnB);
ZenButton zen_C(M5.BtnC);
GpsConnector gps(gnss, Serial2);
// Make GPS connector globally available for shutdown routine
GpsConnector* g_active_gps = &gps;
NavsatCollector navsat(gnss, gps.get_data());
GeigerCounter gm_sensor;
BatteryIndicator battery_indicator;
DateTimeProvider rtc;
ShakeDetector shake_detector;
LogAggregator log_aggregator(settings);
ConfigWebServer config_server(settings);
SoundManager sound_manager;
BatteryLedIndicator battery_led_indicator;

// Data handlers
SdLogger journal_logger(settings, SdLogger::journal);
SdLogger drive_logger(settings, SdLogger::drive);
SdLogger survey_logger(settings, SdLogger::survey);
SdLogger flight_logger(settings, SdLogger::flight); // Using dedicated flight log type
BluetoothReporter bt_connector(settings);
ApiConnector api_connector(settings);

// Supervisors
GFXScreen gfx_screen(settings, controller);

void setup() {
  /// Hardware configurations
  M5.begin();
  // I2C initialized via M5.begin()

  // For Core2: Set CPU to 80MHz early for power savings and WiFi stability
  // WiFi will initialize at 80MHz and stay there (no frequency changes = no corruption)
  #ifndef CONFIG_IDF_TARGET_ESP32S3
    setCpuFrequencyMhz(80);
    M5_LOGI("Core2: CPU set to 80MHz for power savings and WiFi stability");
  #endif

  // Initialize NVS. This is required for WiFi and Preferences.
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      M5_LOGE("NVS: %s. Erasing NVS and retrying...", esp_err_to_name(ret));
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  M5_LOGI("NVS initialized successfully.");

  M5.Log.setLogLevel(m5::log_target_t::log_target_serial, esp_log_level_t::ESP_LOG_DEBUG);

  M5_LOGD("MAIN SETUP DEBUG ENABLED");
  

  // Check SD card status
  M5_LOGI("Checking SD card status...");
  if(!SD.begin(GPIO_NUM_4, SPI, 20000000)){
    M5_LOGE("SD Card Mount Failed");
  } else {
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        M5_LOGE("No SD card attached");
    } else {
        M5_LOGI("SD Card Type: %d", cardType);
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        M5_LOGI("SD Card Size: %lluMB", cardSize);
    }
  }
  /// Software configurations

  M5_LOGD("Register workers...");
  controller.register_worker(k_worker_gps_connector, gps);
  controller.register_worker(k_worker_navsat_collector, navsat);
  controller.register_worker(k_worker_gm_sensor, gm_sensor);
  controller.register_worker(k_worker_shake_detector, shake_detector);
  controller.register_worker(k_worker_battery_indicator, battery_indicator);
  controller.register_worker(k_worker_rtc_connector, rtc);
  controller.register_worker(k_worker_battery_led_indicator, battery_led_indicator);
  controller.register_worker(k_worker_button_3, zen_A);
  controller.register_worker(k_worker_button_2, zen_B);
  controller.register_worker(k_worker_button_1, zen_C);
  controller.register_worker(k_worker_log_aggregator, log_aggregator);
  controller.register_worker(k_worker_device_state, controller);
  controller.register_worker(k_worker_local_storage, settings);
  controller.register_worker(k_worker_config_server, config_server);
  controller.register_worker(k_worker_sound_manager, sound_manager);

  M5_LOGD("Register handlers...");
  controller.register_handler(k_handler_journal_logger, journal_logger);
  controller.register_handler(k_handler_drive_logger, drive_logger);
  controller.register_handler(k_handler_survey_logger, survey_logger);
  controller.register_handler(k_handler_flight_logger, flight_logger);
  controller.register_handler(k_handler_bluetooth_reporter, bt_connector);
  controller.register_handler(k_handler_api_reporter, api_connector);

  M5_LOGD("Register supervisors...");
  controller.register_supervisor(gfx_screen);

  M5_LOGD("Start default workers...");
  controller.start_default_workers();

  M5_LOGD("Setup complete");
}

void loop() {
  // checkUblox() drains the entire Serial2 buffer looking for UBX packets.
  // In NMEA fallback mode the GPS sends NMEA sentences, not UBX — skip it so
  // that produceDataNmea() can read those bytes.
  if (gps.active() && !gps.get_data().nmea_mode) {
    gnss.checkUblox();
  }

  M5.update();
  
  // Establish current time for subsequent logic
  uint32_t current_time = millis();
  
  // Direct M5Unified power button (BtnPWR) logging + GPS backup on click (1.5s cooldown)
  {
    static bool pwr_prev = false;
    static uint32_t last_pwr_backup = 0;
    bool pwr_now = M5.BtnPWR.isPressed();
    if (pwr_now && !pwr_prev) {
      M5_LOGI("[BtnPWR] pressed");
    } else if (!pwr_now && pwr_prev) {
      M5_LOGI("[BtnPWR] released");
    }
    pwr_prev = pwr_now;
    // Trigger on click with cooldown
    if (M5.BtnPWR.wasClicked()) {
      if (current_time - last_pwr_backup >= 1500) {
        M5_LOGI("[BtnPWR] clicked — saving GPS data to NVS...");
        bool ok = gps.backupGpsMemoryToNVS();
        if (ok) {
          M5_LOGI("[BtnPWR] GPS data saved to NVS");
        } else {
          M5_LOGE("[BtnPWR] GPS save FAILED");
        }
        // Also save a warm-start seed to SD for boot-time aiding
        bool sd_ok = gps.saveWarmStartSeedToSD();
        if (sd_ok) {
          M5_LOGI("[BtnPWR] GNSS warm-start seed saved to SD");
        } else {
          M5_LOGW("[BtnPWR] GNSS warm-start seed NOT saved to SD (no valid data or SD error)");
        }
        // Optionally dump the full GNSS database to SD for host-side restore
        bool dbd_ok = gps.dumpDatabaseToSD();
        if (dbd_ok) {
          M5_LOGI("[BtnPWR] GNSS database dump saved to SD");
        } else {
          M5_LOGW("[BtnPWR] GNSS database dump not saved (timeout or SD error)");
        }
        last_pwr_backup = current_time;
      }
    }
  }

  // Use long press on Button A (menu button) for sound toggle
  static uint32_t last_toggle_time = 0;
  static uint32_t button_a_press_start = 0;
  static bool button_a_long_press_detected = false;
  
  // Check for Button A long press (2 seconds) for sound toggle
  if (M5.BtnA.isPressed()) {
    // If this is the start of a press, record the time
    if (button_a_press_start == 0) {
      button_a_press_start = current_time;
      M5_LOGD("Button A press started");
    }
    
    // Check for long press (2 seconds) and trigger only once per press
    if (!button_a_long_press_detected && 
        (current_time - button_a_press_start >= 2000)) {
      button_a_long_press_detected = true;
      
      // Only toggle if enough time has passed since last toggle (debounce)
      if (current_time - last_toggle_time > 1000) {
        M5_LOGI("===== BUTTON A LONG PRESS DETECTED =====");
        
        // Toggle sound
        sound_manager.toggleSound();
        
        // Update last toggle time for debounce protection
        last_toggle_time = current_time;
        
        // Debug output
        M5_LOGI("Sound toggled with Button A long press: %s", sound_manager.isSoundEnabled() ? "ON" : "OFF");
      }
    }
  } else {
    // Button A released, reset tracking variables
    if (button_a_press_start > 0) {
      M5_LOGD("Button A released after %lu ms", (unsigned long)(current_time - button_a_press_start));
      button_a_press_start = 0;
      button_a_long_press_detected = false;
    }
  }

  controller.run();
}
