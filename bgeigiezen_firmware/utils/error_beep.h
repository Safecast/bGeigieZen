#ifndef BGEIGIEZEN_ERROR_BEEP_H_
#define BGEIGIEZEN_ERROR_BEEP_H_

#include <Worker.hpp>
#include "identifiers.h"
#include "workers/sound_manager.h"
#include "user_config.h"

/**
 * Utility function to play error beeps when displaying error messages
 * This function attempts to find the SoundManager worker and play error beeps
 * @param workers The worker map containing all system workers
 */
inline void playErrorBeepsIfAvailable(const WorkerMap& workers) {
  try {
    // Try to get the SoundManager worker
    auto sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);
    if (sound_manager != nullptr) {
      sound_manager->playErrorBeeps();
    }
  } catch (...) {
    // Silently ignore if SoundManager is not available
    // This ensures error messages still display even if sound system fails
  }
}

/**
 * Convenience function to set error text color and play beeps
 * @param workers The worker map containing all system workers
 */
inline void setErrorColorWithBeep(const WorkerMap& workers) {
  M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
  playErrorBeepsIfAvailable(workers);
}

#endif // BGEIGIEZEN_ERROR_BEEP_H_
