#include <numeric>
#include "gm_sensor.h"
#include "debugger.h"

GeigerCounter::GeigerCounter() : Worker<GeigerData>(), pulse_counter() {
  std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
}

bool GeigerCounter::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  pulse_counter.begin();
  return true;
}

int8_t GeigerCounter::produce_data() {
  if (!pulse_counter.available()) {
    return e_worker_idle;
  }

  data.cps = pulse_counter.get_last_count();

  // increase total count
  data.total += data.cps;

  if (data.cps == 0 && data.total == 0) {
    // Don't start recording until anything has been read.
    return e_worker_idle;
  }

  // update the shift register
  _pos = (_pos + 1) % GEIGER_AVERAGING_N_BINS;
  if (_pos == 0 && data.cpm_raw > 0) {
    data.valid = true;
  }
  _shift_reg[_pos] = data.cps;

  // sum up the shift register
  data.cpm_raw = std::accumulate(_shift_reg.begin(), _shift_reg.end(), 0u);

  data.cp5s = _shift_reg[(_pos) % GEIGER_AVERAGING_N_BINS]
      + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 1) % GEIGER_AVERAGING_N_BINS]
      + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 2) % GEIGER_AVERAGING_N_BINS]
      + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 3) % GEIGER_AVERAGING_N_BINS]
      + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 4) % GEIGER_AVERAGING_N_BINS];

  // CPM compensated for deadtime (medcom international)
  data.cpm_comp = static_cast<uint32_t>(static_cast<float>(data.cpm_raw) / (1 - (static_cast<float>(data.cpm_raw) * 1.8833e-6)));

  // peak measurement
  if (data.cpm_comp > data.cpm_comp_peak) {
    data.cpm_comp_peak = data.cpm_comp;
  }

  // micro-Sieverts per hour conversion
  data.uSv = static_cast<float>(data.cpm_comp) * _ush_factor;
  data.Bqm2 = static_cast<float>(data.cpm_comp) * _bqm2_factor;

  data.uSv_sec = static_cast<float>(data.cps) * _ush_factor * GEIGER_AVERAGING_N_BINS;
  data.Bqm2_sec = static_cast<float>(data.cps) * _bqm2_factor * GEIGER_AVERAGING_N_BINS;

  data.uSv_5sec = static_cast<float>(data.cp5s) * _ush_factor * (static_cast<float>(GEIGER_AVERAGING_N_BINS) / 12);
  data.Bqm2_5sec = static_cast<float>(data.cp5s) * _bqm2_factor * (static_cast<float>(GEIGER_AVERAGING_N_BINS) / 12);

  data.alert = data.cpm_comp > _cpm_alert_level;

  return e_worker_data_read;
}
