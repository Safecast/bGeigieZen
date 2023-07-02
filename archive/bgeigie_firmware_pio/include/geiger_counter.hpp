#ifndef __GEIGER_COUNTER_HPP__
#define __GEIGER_COUNTER_HPP__

#include <array>

#include <M5Core2.h> // #include <M5Stack.h>

#include <config.hpp>
#include <hardwarecounter.hpp>

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
  void update();
};

#endif // __GEIGER_COUNTER_HPP__
