// Test code from Average CPM count with sliding window for bGeigeiRaku. Code taken from bGeigieNano and Pointcast.

#include <M5Stack.h>
#include <hardwarecounter.hpp>
#include <gps.hpp>
#include <display.hpp>
#include <config.hpp>

// Peripherals setup
// - pulse counter with 12 bin moving average
HardwareCounter pulse_counter(GEIGER_AVERAGING_PERIOD_MS, GEIGER_PULSE_GPIO);
GeigerMeasurement geiger_count(GEIGER_SENSOR1_CPM_FACTOR);
// - GPS sensor
GPSSensor gps(GPS_SERIAL_NUM, GPS_BAUD_RATE);

Display display(LCD_REFRESH_RATE);

void setup()
{
  //Serial setup for M5Stack
  M5.begin();
  Serial.begin(115200);
  Serial.println("test");

  display.setup();

  // Reset the pulse counter before starting
  pulse_counter.reset();
}

void loop()
{

  // Run all the non-blocking update routines here
  gps.update();
  pulse_counter.update();

  if (gps.available()) {
    // do the stuff we want to do upon GPS update here
    display.update(gps);
  }

  // Process the new geiger count when available
  if (pulse_counter.available()) {
    // update the Geiger counter with the new measurement
    geiger_count.feed(pulse_counter.get_last_count());
    // immediately update the display
    display.update(geiger_count);
  }

}
