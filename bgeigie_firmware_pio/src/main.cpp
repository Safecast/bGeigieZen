// Test code from Average CPM count with sliding window for bGeigeiRaku. Code
// taken from bGeigieNano and Pointcast.

#include <M5Stack.h>

#include <battery.hpp>
#include <config.hpp>
#include <display.hpp>
#include <fsm.hpp>
#include <gps.hpp>
#include <hardwarecounter.hpp>
#include <logger.hpp>
#include <sd_wrapper.hpp>
#include <setup.hpp>

// Copied from
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/main.cpp
#ifndef CONFIG_ARDUINO_LOOP_STACK_SIZE
#define CONFIG_ARDUINO_LOOP_STACK_SIZE 8192
#endif

#ifndef CONFIG_LOGGING_LOOP_RUNNING_CORE
#define CONFIG_LOGGING_LOOP_RUNNING_CORE 0
#endif
#ifndef CONFIG_DISPLAY_LOOP_RUNNING_CORE
#define CONFIG_DISPLAY_LOOP_RUNNING_CORE 1
#endif

// This the state machine that will drive all the logic
FiniteStateMachine fsm;

// object giving access to persistent setup of the device
Setup device_setup;

// Peripherals setup
// - pulse counter with 12 bin moving average
GeigerCounter geiger_count(GEIGER_AVERAGING_PERIOD_S, GEIGER_PULSE_GPIO);
// - GPS sensor
GPSSensor gps(GPS_SERIAL_NUM, GPS_BAUD_RATE);
// - Battery Gauge
BatteryMonitorIP5306 battery_monitor;

// Data sinks
BGeigieLogFormatter bgeigie_formatter;
SDWrapper sd_wrapper;
SDLogger logger;
Display display(LCD_REFRESH_RATE);

void on_geiger_counter_available() {
  // mark the Geiger data as not fresh
  geiger_count.consume();

  // immediately update the data consummers
  bgeigie_formatter.feed(geiger_count);
  display.feed(geiger_count);
  display.feed_battery_level(battery_monitor.get_level());
}

void on_gps_available() {
  display.feed(gps);
  bgeigie_formatter.feed(gps);
}

void logging_loop(void *arg);
void display_loop(void *arg);

void setup() {
  // we put a delay at the beginning so that we can open the serial for debug
  delay(5000);

  // Serial setup for M5Stack
  M5.begin();
  Serial.begin(115200);

  // Start I2C communications for battery level indicator
  Wire.begin();

  // start the SD card for the logger and the configuration file
  auto ret = sd_wrapper.begin();
  while (!ret) {
    Serial.println("SD error");
    delay(1000);
  }

  // Once we have initialized the SD card, we can read the configuration of the
  // device
  device_setup.initialize();
  if (sd_wrapper.ready()) {
    bool success = device_setup.load_from_file(SETUP_FILENAME);
    if (!success) Serial.println("Failed to read config from file");
  }

  // Now we can setup the device ID in the logger
  geiger_count.configure(device_setup.config().cpm2ush_divider,
                         device_setup.config().cpm2bqm2_factor,
                         device_setup.config().alarm_level);
  bgeigie_formatter.set_device_id(device_setup.config().device_id);
  display.feed(device_setup);

  // Reset the pulse counter before starting
  geiger_count.begin();

  fsm.register_event(EVENT_SETUP_FINISHED);

  // Start logging and display tasks on separate cores
  /*
  xTaskCreatePinnedToCore(logging_loop, "LoggingTask",
                          CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 1, NULL,
                          CONFIG_LOGGING_LOOP_RUNNING_CORE);
  delay(10);
  xTaskCreatePinnedToCore(display_loop, "DisplayTask",
                          CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 1, NULL,
                          CONFIG_DISPLAY_LOOP_RUNNING_CORE);
  delay(10);
  */
}

void logging_loop(void *arg) {
  // Run all the non-blocking update routines here
  M5.update();       // updates buttons and stuff
  geiger_count.update();
  gps.update();      // reads from the GPS
  display.update();  // redraws the display and manages the states

  // This the bGeigie logger state machine
  bool ret = false;

  switch (fsm.get_state()) {
    case BG_STARTUP:
      fsm.register_event(EVENT_SETUP_FINISHED);
      break;

    case BG_GPS_TIME_NOT_ACQUIRED:
      if (geiger_count.available()) on_geiger_counter_available();

      if (gps.available()) {
        on_gps_available();

        if (gps.time_valid()) fsm.register_event(EVENT_GPS_DATE_BECAME_CORRECT);
      }
      break;

    case BG_CREATE_LOG_FILE:
      ret =
          logger.start(device_setup.config().device_id, gps.data().date.year(),
                       gps.data().date.month(), gps.data().date.day());
      if (!ret)
        Serial.println("Log creation failed");
      else
        Serial.println("Log creation succeeded");
      fsm.register_event(EVENT_LOG_FILE_CREATED);
      break;

    case BG_LOGGING:
      if (geiger_count.available()) on_geiger_counter_available();

      if (gps.available()) on_gps_available();

      if (bgeigie_formatter.ready()) {
        Serial.println(bgeigie_formatter.format());
        if (!sd_wrapper.ready())
          Serial.println("SD card could not be started");
        else if (!logger.folder_created()) {
          Serial.println("Folder creation failed.");
          ret = logger.start(device_setup.config().device_id,
                             gps.data().date.year(), gps.data().date.month(),
                             gps.data().date.day());
        } else if (!logger.ready())
          Serial.println("Log creation failed");
        else {
          ret = logger.log(bgeigie_formatter.format());

          if (!ret) Serial.println("Log to sd card failed");
        }
      }
      break;
  }
}

void display_loop(void *arg) {
  // Run all the non-blocking update routines here
  // display.update();
}

void loop() { logging_loop(NULL); }
