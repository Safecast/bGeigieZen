/*
 *  Provides GeigerCounter, a class that wraps around the ESP32 hardware counter
 *  and also does moving average, peak detection, CPM conversion, etc
 */
#ifndef __HARDWARECOUNTER_H__
#define __HARDWARECOUNTER_H__

#include <M5Stack.h>
#include <Ticker.h>
#include <driver/pcnt.h>

#include <algorithm>
#include <array>
#include <config.hpp>
#include <limits>

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

class GeigerCounter {
 private:
  HardwareCounter pulse_counter;
  float _ush_factor = 1.0 / SETUP_DEFAULT_USH_DIVIDER;
  float _bqm2_factor =
      SETUP_DEFAULT_BQM2_FACTOR;  // default factor for surface measurements
  float _cpm2ush_divider = SETUP_DEFAULT_USH_DIVIDER;  // default for pancake
  uint32_t _cpm_alarm_level = SETUP_DEFAULT_ALARM_LEVEL;

  int _pos = 0;  // current position in shift register
  std::array<uint32_t, GEIGER_AVERAGING_N_BINS> _shift_reg;

  bool _available = false;
  bool _valid = false;
  uint32_t _cpb = 0;
  uint32_t _cpm_raw = 0;
  uint32_t _cpm_comp = 0;
  uint32_t _cpm_comp_peak = 0;
  uint32_t _total = 0;
  float _uSv = 0.0;
  float _Bqm2 = 0.0;

 public:
  GeigerCounter(uint32_t time_interval, int gpio, float cpm2ush_divider,
                float bqm2_factor, uint32_t cpm_alarm_level)
      : pulse_counter(HardwareCounter(time_interval, gpio)) {
    configure(cpm2ush_divider, bqm2_factor, cpm_alarm_level);
    // fill the shift register with zeros
    std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
  }
  GeigerCounter(uint32_t time_interval, int gpio)
      : GeigerCounter(time_interval, gpio, SETUP_DEFAULT_USH_DIVIDER,
                      SETUP_DEFAULT_BQM2_FACTOR, SETUP_DEFAULT_ALARM_LEVEL) {}

  void configure(float cpm2ush_divider, float bqm2_factor,
                 uint32_t cpm_alarm_level) {
    _ush_factor = 1.0 / cpm2ush_divider;
    _bqm2_factor = bqm2_factor;
    _cpm2ush_divider = _cpm2ush_divider;
    _cpm_alarm_level = cpm_alarm_level;
  }

  void begin() { pulse_counter.begin(); }

  uint32_t per_bin() const { return _cpb; }
  uint32_t total() const { return _total; }
  uint32_t peak_per_minute() const { return _cpm_comp_peak; }
  uint32_t per_minute_raw() const { return _cpm_raw; }
  uint32_t per_minute() const { return _cpm_comp; }
  float uSv() const { return _uSv; }
  float uSv_single_bin() const { return _cpb * 12.0 * _ush_factor; }
  float Bqm2() const { return _Bqm2; }
  float Bqm2_single_bin() const { return _cpb * 12.0 * _bqm2_factor; }

  bool alarm() const { return _cpm_comp > _cpm_alarm_level; }

  size_t n_bins() const { return _shift_reg.size(); }

  bool valid() const { return _valid; }  // indicates if all bins are filled
  bool available() const { return _available; }
  void consume() { _available = false; }

  void update() {
    if (!pulse_counter.available()) return;

    _cpb = pulse_counter.get_last_count();

    // increase total count
    _total += _cpb;

    // update the shift register
    _pos++;
    if (_pos == _shift_reg.size()) {
      _pos = 0;
      if (!_valid) _valid = true;
    }
    _shift_reg[_pos] = _cpb;

    // sum up the shift register
    _cpm_raw = std::accumulate(_shift_reg.begin(), _shift_reg.end(), 0);

    // CPM compensated for deadtime (medcom international)
    _cpm_comp =
        (uint32_t)((float)_cpm_raw / (1 - (((float)_cpm_raw * 1.8833e-6))));

    // peak measurement
    if (_cpm_comp > _cpm_comp_peak) _cpm_comp_peak = _cpm_comp;

    // micro-Sieverts per hour conversion
    _uSv = _cpm_comp * _ush_factor;
    _Bqm2 = _cpm_comp * _bqm2_factor;

    _available = true;
  }
};

#endif  // __HARDWARECOUNTER_H__
