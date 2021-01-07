// Test code from Average CPM count with sliding window for bGeigeiRaku. Code
// taken from bGeigieNano and Pointcast.

#include <M5Stack.h>

#include <config.hpp>
#include <display.hpp>
#include <fsm.hpp>
#include <gps.hpp>
#include <hardwarecounter.hpp>
#include <logger.hpp>
#include <battery.hpp>
#include <sd_wrapper.hpp>

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

// Peripherals setup
// - pulse counter with 12 bin moving average
HardwareCounter pulse_counter(GEIGER_AVERAGING_PERIOD_MS, GEIGER_PULSE_GPIO);
GeigerMeasurement geiger_count(GEIGER_SENSOR1_CPM_FACTOR);
// - GPS sensor
GPSSensor gps(GPS_SERIAL_NUM, GPS_BAUD_RATE);
// - Battery Gauge
BatteryMonitorIP5306 battery_monitor;

// Data sinks
BGeigieLogFormatter bgeigie_formatter(DEVICE_ID);
SDWrapper sd_wrapper;
SDLogger logger;
Display display(LCD_REFRESH_RATE);

void on_pulse_counter_available() {
  // update the Geiger counter with the new measurement
  geiger_count.feed(pulse_counter.get_last_count());

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

  // Reset the pulse counter before starting
  pulse_counter.reset();

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
  M5.update();             // updates buttons and stuff
  display.update();        // redraws the display and manages the states
  gps.update();            // reads from the GPS
  pulse_counter.update();  // checks the pulse counter

  // This the bGeigie logger state machine
  bool ret = false;

  switch (fsm.get_state()) {
    case BG_STARTUP:
      fsm.register_event(EVENT_SETUP_FINISHED);
      break;

    case BG_GPS_TIME_NOT_ACQUIRED:
      if (pulse_counter.available()) on_pulse_counter_available();

      if (gps.available()) {
        on_gps_available();

        if (gps.time_valid()) fsm.register_event(EVENT_GPS_DATE_BECAME_CORRECT);
      }
      break;

    case BG_CREATE_LOG_FILE:
      ret = logger.start(DEVICE_ID, gps.data().date.year(),
                         gps.data().date.month(), gps.data().date.day());
      if (!ret)
        Serial.println("Log creation failed");
      else
        Serial.println("Log creation succeeded");
      fsm.register_event(EVENT_LOG_FILE_CREATED);
      break;

    case BG_LOGGING:
      if (pulse_counter.available()) on_pulse_counter_available();

      if (gps.available()) on_gps_available();

      if (bgeigie_formatter.ready()) {
        Serial.println(bgeigie_formatter.format());
        if (!sd_wrapper.ready())
          Serial.println("SD card could not be started");
        else if (!logger.folder_created()) {
          Serial.println("Folder creation failed.");
          ret = logger.start(DEVICE_ID, gps.data().date.year(),
                             gps.data().date.month(), gps.data().date.day());
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
