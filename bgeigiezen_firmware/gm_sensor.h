#ifndef BGEIGIEZEN_GEIGERSENSOR_HPP
#define BGEIGIEZEN_GEIGERSENSOR_HPP

#include <Worker.hpp>
#include "hardwarecounter.hpp"

struct GeigerData {
  bool available = false;
  bool valid = false;
  uint32_t cpb = 0;
  uint32_t cpm_raw = 0;
  uint32_t cpm_comp = 0;
  uint32_t cpm_comp_peak = 0;
  uint32_t total = 0;
  float uSv = 0.0;
  float Bqm2 = 0.0;
};

/**
 * Geiger counter worker, produces CPM among other data.
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
  float _cpm2ush_divider = SETUP_DEFAULT_USH_DIVIDER;  // default for pancake
  uint32_t _cpm_alert_level = SETUP_DEFAULT_ALERT_LEVEL;

  int _pos = 0;  // current position in shift register
  std::array<uint32_t, GEIGER_AVERAGING_N_BINS> _shift_reg;


};

#endif //BGEIGIEZEN_GEIGERSENSOR_HPP
