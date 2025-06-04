/**
 * The bGeigieZen is the next generation, modern Open Source customizable Geiger Counter. The followup to the
 * successful bGeigieNanoKit.
 *
 * Homepage: https://bgeigiezen.safecast.jp/
 * GitHub https://github.com/Safecast/bGeigieZen
 * wiki: https://github.com/Safecast/bGeigieZen/wiki
 * Slack channel: #bgeigiezen
 *
 *  Copyright (c) 2025, Safecast

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
#include "screens/base_screen.h"
#include "screens/boot_screen.h"
#include "screens/default_entry_screen.h"
#include "screens/file_browser.h"
#include "screens/fixed_mode.h"
#include "screens/flight_mode.h"
#include "screens/first_time_startup.h"

#include "screens/log_viewer.h"
#include "screens/menu_window.h"
#include "screens/satellite_view.h"
#include "screens/sd_message.h"
#include "screens/sd_wipe.h"
#include "screens/zen_info.h"
#include "screens/debug_mode.h"
#include "screens/drive_mode.h"
#include "screens/survey_mode.h"
#include "utils/power_manager.h"

#include "gfx_screen.h"
#include "handlers/api_connector.h"
#include "handlers/bluetooth_reporter.h"
#include "handlers/sd_logger.h"
#include "handlers/debug_logger.h"
#include "identifiers.h"
#include "handlers/battery_logger.h"
#include "workers/battery_indicator.h"
#include "workers/configuration_server.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"
#include "workers/navsat_collector.h"
#include "workers/rtc_connector.h"
#include "workers/shake_detector.h"
#include "workers/zen_button.h"
#include "workers/sound_manager.h"

TeenyUbloxConnect gnss;
LocalStorage settings;
Controller controller(settings, gnss);

// Workers
ZenButton zen_A(M5.BtnA);
ZenButton zen_B(M5.BtnB);
ZenButton zen_C(M5.BtnC);
GpsConnector gps(gnss, Serial2);
NavsatCollector navsat(gnss);
GeigerCounter gm_sensor;
BatteryIndicator battery_indicator;
DateTimeProvider rtc;
ShakeDetector shake_detector;
LogAggregator log_aggregator(settings);
ConfigWebServer config_server(settings);
SoundManager sound_manager;
BatteryLogger battery_logger(settings);

// Data handlers
SdLogger journal_logger(settings, SdLogger::journal);
SdLogger drive_logger(settings, SdLogger::drive);
SdLogger survey_logger(settings, SdLogger::survey);
SdLogger flight_logger(settings, SdLogger::flight); // Using dedicated flight log type
GpsDebugLogger gps_debug_logger(settings, gnss);
BluetoothReporter bt_connector(settings);
ApiConnector api_connector(settings);

// Supervisors
GFXScreen gfx_screen(settings, controller);

void setup() {
    // Initialize power manager with controller
    PowerManager::instance().setController(&controller);
    PowerManager::setBatteryLogger(&battery_logger);
    PowerManager::setSettings(&settings);

  /// Hardware configurations
  M5.begin();

  M5.Log.setLogLevel(m5::log_target_t::log_target_serial, esp_log_level_t::ESP_LOG_DEBUG);

  M5_LOGD("MAIN SETUP DEBUG ENABLED");

  /// Software configurations

  M5_LOGD("Register workers...");
  controller.register_worker(k_worker_gps_connector, gps);
  controller.register_worker(k_worker_navsat_collector, navsat);
  controller.register_worker(k_worker_gm_sensor, gm_sensor);
  controller.register_worker(k_worker_shake_detector, shake_detector);
  controller.register_worker(k_worker_battery_indicator, battery_indicator);
  controller.register_worker(k_worker_rtc_connector, rtc);
  controller.register_worker(k_worker_button_3, zen_A);
  controller.register_worker(k_worker_button_2, zen_B);
  controller.register_worker(k_worker_button_1, zen_C);
  controller.register_worker(k_worker_log_aggregator, log_aggregator);
  controller.register_worker(k_worker_device_state, controller);
  controller.register_worker(k_worker_local_storage, settings);
  controller.register_worker(k_worker_config_server, config_server);
  controller.register_worker(k_worker_sound_manager, sound_manager);
  controller.register_handler(k_handler_battery_logger, battery_logger);

  M5_LOGD("Register handlers...");
  controller.register_handler(k_handler_journal_logger, journal_logger);
  controller.register_handler(k_handler_drive_logger, drive_logger);
  controller.register_handler(k_handler_survey_logger, survey_logger);
  controller.register_handler(k_handler_flight_logger, flight_logger);
  controller.register_handler(k_handler_gps_debug_logger, gps_debug_logger);
  controller.register_handler(k_handler_bluetooth_reporter, bt_connector);
  controller.register_handler(k_handler_api_reporter, api_connector);

  M5_LOGD("Register supervisors...");
  controller.register_supervisor(gfx_screen);

  M5_LOGD("Start default workers...");
  controller.start_default_workers();

  M5_LOGD("Setup complete");
}

void loop() {
  if (gps.active()) {
    gnss.checkUblox();
  }

  M5.update();
  
  // Use long press on Button A (menu button) for sound toggle
  static uint32_t last_toggle_time = 0;
  static uint32_t button_a_press_start = 0;
  static bool button_a_long_press_detected = false;
  uint32_t current_time = millis();
  
  // Check if Button A is pressed
  if (M5.BtnA.isPressed()) {
    // If this is the start of a press, record the time
    if (button_a_press_start == 0) {
      button_a_press_start = current_time;
      Serial.println("Button A press started");
    }
    
    // Check for long press (2 seconds) and trigger only once per press
    if (!button_a_long_press_detected && 
        (current_time - button_a_press_start >= 2000)) {
      button_a_long_press_detected = true;
      
      // Only toggle if enough time has passed since last toggle (debounce)
      if (current_time - last_toggle_time > 1000) {
        Serial.println("\n===== BUTTON A LONG PRESS DETECTED =====\n");
        
        // Toggle sound
        sound_manager.toggleSound();
        
        // Update last toggle time for debounce protection
        last_toggle_time = current_time;
        
        // Debug output
        Serial.println("Sound toggled with Button A long press: " + 
                       String(sound_manager.isSoundEnabled() ? "ON" : "OFF"));
      }
    }
  } else {
    // Button A released, reset tracking variables
    if (button_a_press_start > 0) {
      Serial.println("Button A released after " + 
                     String(current_time - button_a_press_start) + " ms");
      button_a_press_start = 0;
      button_a_long_press_detected = false;
    }
  }

  // Check power button (Button C)
  if (M5.BtnC.wasPressed()) {
    PowerManager::instance().enterLowPowerMode();
  }

  controller.run();
}
