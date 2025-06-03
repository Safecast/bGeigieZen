#include <numeric>
#include "gm_sensor.h"
#include "utils/ulp_geiger_counter.h"

// Global variable for CPS value that can be accessed by SoundManager
uint16_t g_cps = 0;

GeigerCounter::GeigerCounter(bool use_ulp) : Worker<GeigerData>(), pulse_counter() {
  use_ulp_counter = use_ulp;
  if (use_ulp_counter) {
    ulp_counter = new ULPGeigerCounter();
  }
  std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
}

bool GeigerCounter::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  if (!retry) {
    M5_LOGD("Start pulse counter...");
    if (use_ulp_counter && ulp_counter) {
      // Use correct RTC-capable GPIO for this board
      int rtc_gpio = GEIGER_PULSE_GPIO_CORE2;
      if (M5.getBoard() == m5::board_t::board_M5StackCoreS3SE) rtc_gpio = GEIGER_PULSE_GPIO_CORES3SE;
      if (M5.getBoard() == m5::board_t::board_M5Stack) rtc_gpio = GEIGER_PULSE_GPIO_COREBASIC;
      ulp_counter->begin(rtc_gpio);
    } else {
      pulse_counter.begin();
    }
  }

  if (use_ulp_counter && ulp_counter) {
    return true; // ULP always available after begin
  } else if (pulse_counter.available() && pulse_counter.get_last_count() > 0) {
    return true;
  }
  return false;
}

int8_t GeigerCounter::produce_data() {
  uint16_t pulses = 0;
  if (use_ulp_counter && ulp_counter) {
    pulses = ulp_counter->get_last_count();
  } else {
    if (!pulse_counter.available()) {
      return e_worker_idle;
    }
    pulses = pulse_counter.get_last_count();
  }

  if (pulses == 0 && data.total == 0) {
    // Don't start recording until anything has been read.
    return e_worker_idle;
  }

  data.cps = pulses;
  
  // Update global CPS variable for SoundManager to access
  g_cps = data.cps;

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
  data.uSvh = static_cast<float>(data.cpm_comp) * _ush_factor;
  data.Bqm2 = static_cast<float>(data.cpm_comp) * _bqm2_factor;

  data.uSvh_5sec = static_cast<float>(data.cp5s * 12) * _ush_factor;
  data.Bqm2_5sec = static_cast<float>(data.cp5s * 12) * _bqm2_factor;

  data.alert = data.cpm_comp > _cpm_alert_level;

  return e_worker_data_read;
}
