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
  data.cpm_comp = static_cast<uint32_t>(static_cast<float>(data.cpm_raw) / (1 - (static_cast<float>(data.cpm_raw) * 1.8833e-6)));

  // peak measurement
  if (data.cpm_comp > data.cpm_comp_peak) {
    data.cpm_comp_peak = data.cpm_comp;
  }

//  DEBUG_PRINTF("reg ["
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
//               "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]\n cpm_raw %d,  cpm_comp %d\n",
//               _shift_reg[0],_shift_reg[1],_shift_reg[2],_shift_reg[3],_shift_reg[4],_shift_reg[5],_shift_reg[6],_shift_reg[7],_shift_reg[8],_shift_reg[9],
//               _shift_reg[10],_shift_reg[11],_shift_reg[12],_shift_reg[13],_shift_reg[14],_shift_reg[15],_shift_reg[16],_shift_reg[17],_shift_reg[18],_shift_reg[19],
//               _shift_reg[20],_shift_reg[21],_shift_reg[22],_shift_reg[23],_shift_reg[24],_shift_reg[25],_shift_reg[26],_shift_reg[27],_shift_reg[28],_shift_reg[29],
//               _shift_reg[30],_shift_reg[31],_shift_reg[32],_shift_reg[33],_shift_reg[34],_shift_reg[35],_shift_reg[36],_shift_reg[37],_shift_reg[38],_shift_reg[39],
//               _shift_reg[40],_shift_reg[41],_shift_reg[42],_shift_reg[43],_shift_reg[44],_shift_reg[45],_shift_reg[46],_shift_reg[47],_shift_reg[48],_shift_reg[49],
//               _shift_reg[50],_shift_reg[51],_shift_reg[52],_shift_reg[53],_shift_reg[54],_shift_reg[55],_shift_reg[56],_shift_reg[57],_shift_reg[58],_shift_reg[59],
//               data.cpm_raw, data.cpm_comp
//  );

  // micro-Sieverts per hour conversion
  data.uSv = static_cast<float>(data.cpm_comp) * _ush_factor;
  data.Bqm2 = static_cast<float>(data.cpm_comp) * _bqm2_factor;

  data.uSv_bin = static_cast<float>(data.cpb) * _ush_factor * GEIGER_AVERAGING_N_BINS;
  data.Bqm2_bin = static_cast<float>(data.cpb) * _bqm2_factor * GEIGER_AVERAGING_N_BINS;
  data.alert = data.cpm_comp > _cpm_alert_level;
  return e_worker_data_read;
}
