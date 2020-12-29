/*
 *  Provides GeigerCounter, a class that wraps around the ESP32 hardware counter
 *  and also does moving average, peak detection, CPM conversion, etc
 */
#ifndef __HARDWARECOUNTER_H__
#define __HARDWARECOUNTER_H__

#include <array>
#include <algorithm>
#include <limits>

#include <driver/pcnt.h>
#include <M5Stack.h>

#include <config.hpp>

class GeigerMeasurement {
  private:
    float _cpm_factor = 340.0;  // default for pancake
    float _conversion_coefficient = 1.0 / 340.0;

    int _pos = 0;  // current position in shift register
    std::array<uint32_t, GEIGER_AVERAGING_N_BINS> _shift_reg;

    uint32_t _cpb = 0;
    uint32_t _cpm_raw = 0;
    uint32_t _cpm_comp = 0;
    uint32_t _cpm_comp_peak = 0;
    uint32_t _total = 0;
    float _uSv = 0.0;

  public:
    GeigerMeasurement(float cpm_factor):
      _cpm_factor(cpm_factor), _conversion_coefficient(1.0 / cpm_factor) {
      // fill the shift register with zeros
      std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
    }

    uint32_t per_bin() const { return _cpb; }
    uint32_t total() const { return _total; }
    uint32_t peak_per_minute() const { return _cpm_comp_peak; }
    uint32_t per_minute_raw() const { return _cpm_raw; }
    uint32_t per_minute() const { return _cpm_comp; }
    float uSv() const { return _uSv; }
    size_t n_bins() const { return _shift_reg.size(); }

    void feed(uint32_t new_cpb) {
      _cpb = new_cpb;

      // increase total count
      _total += new_cpb;

      // update the shift register
      _pos++;
      if (_pos == _shift_reg.size())
        _pos = 0;
      _shift_reg[_pos] = new_cpb;

      // sum up the shift register
      _cpm_raw = std::accumulate(_shift_reg.begin(), _shift_reg.end(), 0);

      // CPM compensated for deadtime (medcom international)
      _cpm_comp = (uint32_t)((float)_cpm_raw / (1 - (((float)_cpm_raw * 1.8833e-6))));

      // peak measurement
      if (_cpm_comp > _cpm_comp_peak)
        _cpm_comp_peak = _cpm_comp;

      // micro-Sieverts per hour conversion
      _uSv = _cpm_comp * _conversion_coefficient;
    }
};

class HardwareCounter {
  private:
    uint32_t _delay = 5000;  // default at 5 seconds
    int _gpio = 2;
    pcnt_unit_t _unit = PCNT_UNIT_0;

    const int16_t _max_value = std::numeric_limits<int16_t>::max();

    uint32_t _n_wraparound = 0;
    uint32_t _start_time;
    uint32_t _last_count = 0;

    bool _available = false;

    uint32_t _get_count_reset() {
      // compute current value of counter and reset
      int16_t count = 0;

      // get the value of the hardware counter
      esp_err_t ret = pcnt_get_counter_value(_unit, &count);
      if (ret != ESP_OK)
        Serial.println("A problem occured in the hardware counter");

      // compute the total value taking into account the wrap-arounds
      uint32_t total_count = _n_wraparound * _max_value + count;

      // reset the counter
      reset();

      return total_count;
    }

  public:

    HardwareCounter(uint32_t time_interval, int gpio, pcnt_unit_t pcnt_unit = PCNT_UNIT_0);

    uint32_t get_last_count() {
      _available = false;  // remove flag after consumption
      return _last_count;
    }

    //sliding windows setup
    void reset();
    void update();
    bool available() { return _available; }

};

#endif // __HARDWARECOUNTER_H__
