#include <algorithm>
#include <numeric> 
#include <geiger_counter.hpp>

void GeigerCounter::update() {
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
