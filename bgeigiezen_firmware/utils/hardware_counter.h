/*
 *  Provides GeigerCounter, a class that wraps around the ESP32 hardware counter
 *  and also does moving average, peak detection, CPM conversion, etc
 */
#ifndef __HARDWARECOUNTER_H__
#define __HARDWARECOUNTER_H__

#include <limits>

#include <Ticker.h>
#include <driver/pcnt.h>

#include "user_config.h"

/**
 * Counts pulses coming in over gpio
 */
class HardwareCounter {
 public:
  HardwareCounter();

  uint32_t get_last_count();

  // sliding windows setup
  void begin();
  void reset();
  bool available() const;

 private:
  inline uint32_t _get_count_reset();

  /* let the timer interrupt routine be a friend */
  friend void timer_intr_handler(void *arg);


  Ticker _timer;

  uint32_t _delay_s;
  int _gpio;
  pcnt_unit_t _unit;
  const int16_t _max_value;
  uint32_t _n_wraparound;
  uint32_t _last_count;
  uint32_t _start_time;
  bool _available;

};


#endif  // __HARDWARECOUNTER_H__
