/**
 * The bGeigieZen is the next generation, modern Open Source customizable Geiger Counter. The followup to the
 * successful bGeigieNanoKit.
 *
 * Homepage: https://bgeigiezen.safecast.jp/
 * GitHub https://github.com/Safecast/bGeigieZen
 * wiki: https://github.com/Safecast/bGeigieZen/wiki
 * Slack channel: #bgeigiezen
 *
 *  Copyright (c) 2023, Safecast

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

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include <Arduino.h>

#include "identifiers.h"
#include "controller.h"
#include "workers/gps_connector.h"
#include "workers/gm_sensor.h"
#include "workers/zen_button.h"
#include "workers/shake_detector.h"
#include "workers/battery_indicator.h"
#include "workers/log_aggregator.h"
#include "workers/rtc_connector.h"
#include "gfx_screen.h"
#include "debugger.h"

LocalStorage settings;
Controller controller(settings);

void setup() {
  DEBUG_BEGIN();
  DEBUG_PRINTLN("MAIN SETUP DEBUG ENABLED");
  /// Hardware configurations
  Wire.begin();
  M5.begin();

  /// Software configurations
  // Workers
  auto* zen_A = new ZenButton(M5.BtnA);
  auto* zen_B = new ZenButton(M5.BtnB);
  auto* zen_C = new ZenButton(M5.BtnC);
  auto* gps = new GpsConnector();
  auto* gm_sensor = new GeigerCounter();
  auto* battery_indicator = new BatteryIndicator();
  auto* rtc = new RtcConnector();
  auto* shake_detector = new ShakeDetector();
  auto* log_aggregator = new LogAggregator();

  // Data handlers

  // Supervisors
  auto* gfx_screen = new GFXScreen(settings, controller);

  DEBUG_PRINTLN("Register workers...");
  controller.register_worker(k_worker_gps_connector, *gps);
  controller.register_worker(k_worker_gm_sensor, *gm_sensor);
  controller.register_worker(k_worker_shake_detector, *shake_detector);
  controller.register_worker(k_worker_battery_indicator, *battery_indicator);
  controller.register_worker(k_worker_rtc_connector, *rtc);
  controller.register_worker(k_worker_button_3, *zen_A);
  controller.register_worker(k_worker_button_2, *zen_B);
  controller.register_worker(k_worker_button_1, *zen_C);
  controller.register_worker(k_worker_log_aggregator, *log_aggregator);
  controller.register_worker(k_worker_device_state, controller);

  DEBUG_PRINTLN("Register handlers...");
  controller.register_handler(k_handler_log_aggregator, *log_aggregator);
  controller.register_handler(k_handler_local_storage, settings);

  DEBUG_PRINTLN("Register supervisors...");
  controller.register_supervisor(*gfx_screen);

  controller.start_default_workers();
}

void loop() {
  M5.update();
  controller.run();
}
