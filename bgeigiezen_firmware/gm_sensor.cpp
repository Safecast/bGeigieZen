#include <numeric>
#include "gm_sensor.h"
#include "debugger.h"

GeigerCounter::GeigerCounter() : Worker<GeigerData>(), pulse_counter() {
  std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
}

bool GeigerCounter::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  // NOTE: this is currently done in pulse_counter's constructor for testing purposes
//  pulse_counter.begin();
  return true;
}

int8_t GeigerCounter::produce_data() {
  if (!pulse_counter.available()) {
    return e_worker_idle;
  }

  data.cpb = pulse_counter.get_last_count();

  // increase total count
  data.total += data.cpb;

  // update the shift register
  _pos = (_pos + 1) % GEIGER_AVERAGING_N_BINS;
  if (_pos == 0) {
    data.valid = true;
  }
  _shift_reg[_pos] = data.cpb;

  // sum up the shift register
  data.cpm_raw = std::accumulate(_shift_reg.begin(), _shift_reg.end(), 0u);

  // CPM compensated for deadtime (medcom international)
  data.cpm_comp = static_cast<uint16_t>(static_cast<float>(data.cpm_raw) / (1 - ((static_cast<float>(data.cpm_raw) * 1.8833e-6))));

  // peak measurement
  if (data.cpm_comp > data.cpm_comp_peak) data.cpm_comp_peak = data.cpm_comp;

  // micro-Sieverts per hour conversion
  data.uSv = static_cast<float>(data.cpm_comp) * _ush_factor;
  data.Bqm2 = static_cast<float>(data.cpm_comp) * _bqm2_factor;

  data.available = true;
  return e_worker_data_read;
}
