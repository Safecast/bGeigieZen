#ifndef __FSM_CONTEXT_HPP__
#define __FSM_CONTEXT_HPP__

#include <config.hpp>

// data sources
#include <battery.hpp>
#include <geiger_counter.hpp>
#include <gps.hpp>
#include <setup.hpp>

// data sinks
#include <display.hpp>
#include <logger.hpp>
#include <sd_wrapper.hpp>

// state machine stuff
#include <fsm_state_base.hpp>
#include <fsm_state_concrete.hpp>

class State;

/*** Avoid name collision with M5Stack Core 2 UI ***/
namespace bGeigieZen {
  enum class Event;
}

class Context {
  State *current_state = NULL;

 protected:
  // Data sources
  Setup device_setup;  // object giving access to persistent setup of the device
  GeigerCounter geiger_count{GEIGER_AVERAGING_PERIOD_S, GEIGER_PULSE_GPIO};
  GPSSensor gps{GPS_SERIAL_NUM, GPS_BAUD_RATE};
  BatteryMonitorIP5306 battery_monitor;

  // Data sinks
  BGeigieLogFormatter bgeigie_formatter;
  SDWrapper sd_wrapper;
  SDLogger logger;
  Display display{LCD_REFRESH_RATE};

  // Methods
  void setup();  // initialize all the components
  void on_geiger_counter_available();
  void on_gps_available();
  void transition_to(State *next_state, bGeigieZen::Event e);

 public:
  Context() {}
  void begin();
  void loop();

  // States need to be friend
  friend class StateStartup;
  friend class StateWaitGPSTime;
  friend class StateLogging;
};

#endif  // __FSM_CONTEXT_HPP__
