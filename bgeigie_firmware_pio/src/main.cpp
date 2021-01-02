// Test code from Average CPM count with sliding window for bGeigeiRaku. Code
// taken from bGeigieNano and Pointcast.

#include <M5Stack.h>

#include <config.hpp>
#include <display.hpp>
#include <gps.hpp>
#include <logger.hpp>
#include <hardwarecounter.hpp>

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

// Peripherals setup
// - pulse counter with 12 bin moving average
HardwareCounter pulse_counter(GEIGER_AVERAGING_PERIOD_MS, GEIGER_PULSE_GPIO);
GeigerMeasurement geiger_count(GEIGER_SENSOR1_CPM_FACTOR);
// - GPS sensor
GPSSensor gps(GPS_SERIAL_NUM, GPS_BAUD_RATE);

// Data sinks
BGeigieLogFormatter bgeigie_formatter(1);
Display display(LCD_REFRESH_RATE);

void logging_loop(void *arg);
void display_loop(void *arg);

void setup() {
  // Serial setup for M5Stack
  M5.begin();
  Serial.begin(115200);

  // Reset the pulse counter before starting
  pulse_counter.reset();

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
  M5.update();  // updates buttons and stuff
  display.update();  // redraws the display and manages the states
  gps.update();  // reads from the GPS
  pulse_counter.update();  // checks the pulse counter

  if (gps.available()) {
    // do the stuff we want to do upon GPS update here
    display.feed(gps);
    bgeigie_formatter.feed(gps);
  }

  // Process the new geiger count when available
  if (pulse_counter.available()) {
    // update the Geiger counter with the new measurement
    geiger_count.feed(pulse_counter.get_last_count());

    // immediately update the data consummers
    bgeigie_formatter.feed(geiger_count);
    display.feed(geiger_count);
  }

  if (bgeigie_formatter.ready()) {
    Serial.println(bgeigie_formatter.format());
  }
}

void display_loop(void *arg) {
  // Run all the non-blocking update routines here
  //display.update();
}

void loop() {
  logging_loop(NULL);
}
