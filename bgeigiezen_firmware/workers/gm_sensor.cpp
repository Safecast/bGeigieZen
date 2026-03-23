#include <numeric>
#include "gm_sensor.h"
#include "workers/local_storage.h"
#include "workers/sound_manager.h"
#include "identifiers.h"

// Global variable for CPS value that can be accessed by SoundManager
uint32_t g_cps = 0;  // Changed to uint32_t for high count rates

GeigerCounter::GeigerCounter() : ProcessWorker<GeigerData>(), pulse_counter() {
  std::fill(_shift_reg.begin(), _shift_reg.end(), 0);
  _samples_collected = 0;  // Track how many samples we've collected
}

bool GeigerCounter::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  if (!retry) {
    M5_LOGD("Start pulse counter...");
    pulse_counter.begin();
  }

  if (pulse_counter.available() && pulse_counter.get_last_count() > 0) {
    return true;
  }
  return false;
}

int8_t GeigerCounter::produce_data(const worker_map_t& workers) {
  // Get the current alert threshold from LocalStorage
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  if (settings) {
    _cpm_alert_level = settings->get_alert_threshold();
  }
  
  if (!pulse_counter.available()) {
    return e_worker_idle;
  }

  while (pulse_counter.available()) {

    data.cps = pulse_counter.get_last_count();
    
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
    _shift_reg[_pos] = data.cps;
    
    // Track samples collected, but cap at N_BINS + 1 to prevent overflow
    if (_samples_collected <= GEIGER_AVERAGING_N_BINS) {
      _samples_collected++;
    }
    
    // Check if we've completed a full minute of data collection
    if (_samples_collected >= GEIGER_AVERAGING_N_BINS) {
      data.valid = true;
    }

    // sum up the shift register - only count positions with actual data
    if (_samples_collected > GEIGER_AVERAGING_N_BINS) {
      // After the first minute, use standard circular buffer sum
      data.cpm_raw = std::accumulate(_shift_reg.begin(), _shift_reg.end(), 0u);
    } else {
      // During first minute (including when exactly 60 samples), only sum positions that have been written
      data.cpm_raw = 0;
      uint32_t samples_to_sum = (_samples_collected <= GEIGER_AVERAGING_N_BINS) ? _samples_collected : GEIGER_AVERAGING_N_BINS;
      for (uint32_t i = 0; i < samples_to_sum; i++) {
        data.cpm_raw += _shift_reg[i];
      }
    }

    // Calculate cp5s (last 5 seconds)
    if (_samples_collected >= 5) {
      // We have at least 5 seconds of data - use the circular buffer logic
      data.cp5s = _shift_reg[(_pos) % GEIGER_AVERAGING_N_BINS]
          + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 1) % GEIGER_AVERAGING_N_BINS]
          + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 2) % GEIGER_AVERAGING_N_BINS]
          + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 3) % GEIGER_AVERAGING_N_BINS]
          + _shift_reg[(_pos + GEIGER_AVERAGING_N_BINS - 4) % GEIGER_AVERAGING_N_BINS];
    } else {
      // During first few seconds, sum all positions we've written
      data.cp5s = 0;
      for (uint32_t i = 0; i < _samples_collected; i++) {
        data.cp5s += _shift_reg[i];
      }
    }

    // CPM compensated for deadtime (medcom international)
    // During first minute, we need to scale the partial data to estimate full CPM
    float effective_cpm_raw;
    if (_samples_collected < GEIGER_AVERAGING_N_BINS) {
      // Scale up the partial data to estimate a full minute
      effective_cpm_raw = (static_cast<float>(data.cpm_raw) / static_cast<float>(_samples_collected)) * GEIGER_AVERAGING_N_BINS;
    } else {
      // At 60 samples or more, cpm_raw represents a full minute
      effective_cpm_raw = static_cast<float>(data.cpm_raw);
    }
    
    // Deadtime compensation formula is only valid up to about 500k CPM
    // Above that, the denominator becomes negative and causes overflow
    float deadtime_factor = 1.0f - (effective_cpm_raw * 1.8833e-6f);
    if (deadtime_factor > 0.1f) {  // Only apply compensation if denominator is reasonable
      data.cpm_comp = static_cast<uint32_t>(effective_cpm_raw / deadtime_factor);
    } else {
      // At very high count rates, deadtime compensation is not valid
      // Just use raw count (detector is saturated anyway)
      data.cpm_comp = static_cast<uint32_t>(effective_cpm_raw);
    }

    // peak measurement
    if (data.cpm_comp > data.cpm_comp_peak) {
      data.cpm_comp_peak = data.cpm_comp;
    }

    // micro-Sieverts per hour conversion
    // Now cpm_comp is always properly scaled, so we can use it directly
    data.uSvh = static_cast<float>(data.cpm_comp) * _ush_factor;
    data.Bqm2 = static_cast<float>(data.cpm_comp) * _bqm2_factor;

    // For 5-second calculations, scale appropriately
    if (_samples_collected >= 5) {
      // We have at least 5 seconds of data, multiply by 12 to get per-minute rate
      data.uSvh_5sec = static_cast<float>(data.cp5s * 12) * _ush_factor;
      data.Bqm2_5sec = static_cast<float>(data.cp5s * 12) * _bqm2_factor;
    } else if (_samples_collected > 0) {
      // Scale based on actual number of seconds we have
      float scale_factor = 60.0f / static_cast<float>(_samples_collected);
      data.uSvh_5sec = static_cast<float>(data.cp5s) * scale_factor * _ush_factor;
      data.Bqm2_5sec = static_cast<float>(data.cp5s) * scale_factor * _bqm2_factor;
    } else {
      // No data yet
      data.uSvh_5sec = 0;
      data.Bqm2_5sec = 0;
    }

    data.alert = data.cpm_comp > _cpm_alert_level;
    
    // Check for alert state transition (crossing from below to above threshold)
    if (data.alert && !_previous_alert_state) {
      // We just crossed the threshold from below - trigger audible alert
      auto* sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);
      if (sound_manager) {
        sound_manager->playCpmAlert();
        M5_LOGD("CPM threshold breached: %u > %u - playing alert sound", data.cpm_comp, _cpm_alert_level);
      }
    }
    
    // Update previous alert state for next iteration
    _previous_alert_state = data.alert;
  }

  return e_worker_data_read;
}
