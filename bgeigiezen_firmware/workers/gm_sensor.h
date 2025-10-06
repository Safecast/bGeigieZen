#ifndef BGEIGIEZEN_GEIGER_COUNTER_H_
#define BGEIGIEZEN_GEIGER_COUNTER_H_

#include <Worker.hpp>
#include <utils/hardware_counter.h>

struct GeigerData {
  bool valid = false;  // True if accumulated data over 1+ minute
  uint32_t cps = 0;  // Past second - changed to uint32_t for high count rates
  uint32_t cp5s = 0;  // Past 5 seconds - changed to uint32_t for high count rates
  uint32_t cpm_raw = 0;  // Past minute
  uint32_t cpm_comp = 0;  // cpm_raw compensated for medcom deadtime
  uint32_t cpm_comp_peak = 0;  // highest cpm_comp recorded
  uint32_t total = 0;  // Total since initialization
  float uSvh = 0.0;  // uSv/h based on the last minute (using cpm_comp)
  float Bqm2 = 0.0;  // Bq/m² based on the last minute (using cpm_comp)
  float uSvh_5sec = 0.0;  // uSv/h based on the last 5 sec (using cp5s)
  float Bqm2_5sec = 0.0;  // Bq/m² based on the last 5 sec (using cp5s)
  bool alert = false;  // cpm_comp > alert level
};

/**
 * Geiger counter worker, produces CPM among other data (See GeigerData).
 */
class GeigerCounter : public ProcessWorker<GeigerData> {
 public:
  explicit GeigerCounter();
  virtual ~GeigerCounter() = default;

  bool activate(bool retry) override;

  int8_t produce_data(const worker_map_t& workers) override;
  
  /**
   * Get direct access to the pulse counter
   * @return Reference to the pulse counter
   */
  HardwareCounter& getPulseCounter() { return pulse_counter; }
  
 private:
  HardwareCounter pulse_counter;
  float _ush_factor = 1.0 / SETUP_DEFAULT_USH_DIVIDER;
  float _bqm2_factor = SETUP_DEFAULT_BQM2_FACTOR;  // default factor for surface measurements
  uint32_t _cpm_alert_level = SETUP_DEFAULT_ALERT_LEVEL;
  bool _previous_alert_state = false;  // Track previous alert state to detect threshold crossings

  int _pos = 0;  // current position in shift register
  uint32_t _samples_collected = 0;  // Track total number of samples collected
  std::array<uint32_t, GEIGER_AVERAGING_N_BINS> _shift_reg;


};

#endif //BGEIGIEZEN_GEIGER_COUNTER_H_
