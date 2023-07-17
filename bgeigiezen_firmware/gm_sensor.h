#ifndef BGEIGIEZEN_GEIGERSENSOR_HPP
#define BGEIGIEZEN_GEIGERSENSOR_HPP

#include <Worker.hpp>
#include "hardware_counter.h"

struct GeigerData {
  bool valid = false;  // True if accumulated data over 1+ minute
  uint32_t cpb = 0;  // Past second
  uint32_t cpm_raw = 0;  // Total past minute
  uint32_t cpm_comp = 0;  // cpm_raw processed
  uint32_t cpm_comp_peak = 0;  // highest cpm_comp recorded
  uint32_t total = 0;  // Total since initialization
  float uSv = 0.0;  // cpm_comp converted to uSv/h
  float Bqm2 = 0.0;  // cpm_comp converted to Bq/m²
  float uSv_bin = 0.0;  // cpb converted to uSv/h
  float Bqm2_bin = 0.0;  // cpb converted to Bq/m²
  bool alert = false;  // cpm_comp > alert level
};

/**
 * Geiger counter worker, produces CPM among other data (See GeigerData).
 */
class GeigerCounter : public Worker<GeigerData> {
 public:
  explicit GeigerCounter();
  virtual ~GeigerCounter() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;
  
 private:
  HardwareCounter pulse_counter;
  float _ush_factor = 1.0 / SETUP_DEFAULT_USH_DIVIDER;
  float _bqm2_factor = SETUP_DEFAULT_BQM2_FACTOR;  // default factor for surface measurements
  uint32_t _cpm_alert_level = SETUP_DEFAULT_ALERT_LEVEL;

  int _pos = 0;  // current position in shift register
  std::array<uint32_t, GEIGER_AVERAGING_N_BINS> _shift_reg;


};

#endif //BGEIGIEZEN_GEIGERSENSOR_HPP
