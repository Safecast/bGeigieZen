/*
 *  Provides GeigerCounter, a class that wraps around the ESP32 hardware counter
 *  and also does moving average, peak detection, CPM conversion, etc
 */
#ifndef __HARDWARECOUNTER_H__
#define __HARDWARECOUNTER_H__

#include <limits>

#include <M5Core2.h> // #include <M5Stack.h>
#include <Ticker.h>
#include <driver/pcnt.h>

#include <config.hpp>

class HardwareCounter {
 private:
  uint32_t _delay_s = GEIGER_AVERAGING_PERIOD_S;  // default at 5 seconds
  int _gpio = GEIGER_PULSE_GPIO;
  pcnt_unit_t _unit = PCNT_UNIT_0;

  const int16_t _max_value = std::numeric_limits<int16_t>::max();

  uint32_t _n_wraparound = 0;

  uint32_t _start_time;
  uint32_t _last_count = 0;

  bool _available = false;

  Ticker _timer;
  inline uint32_t _get_count_reset();

  /* let the timer interrupt routine be a friend */
  friend void timer_intr_handler(void *arg);

 public:
  HardwareCounter() {}
  HardwareCounter(uint32_t delay_s, int gpio, pcnt_unit_t unit = PCNT_UNIT_0)
      : _delay_s(delay_s), _gpio(gpio), _unit(unit) {}

  uint32_t get_last_count() {
    _available = false;  // remove flag after consumption
    return _last_count;
  }

  // sliding windows setup
  void begin();
  void reset();
  bool available() { return _available; }
};


#endif  // __HARDWARECOUNTER_H__
