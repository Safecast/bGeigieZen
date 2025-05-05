#ifndef BGEIGIEZEN_SOUND_MANAGER_H_
#define BGEIGIEZEN_SOUND_MANAGER_H_

#include <Worker.hpp>
#include <M5Unified.hpp>
#include <Preferences.h>

/**
 * Sound manager for Geiger counter clicks
 * Plays click sounds in response to radiation pulses
 * Can be toggled on/off with the power button (short press)
 */
class SoundManager : public Worker<bool> {
 public:
  explicit SoundManager();
  virtual ~SoundManager() = default;

  bool activate(bool retry) override;
  int8_t produce_data() override;
  
  /**
   * Load sound state from persistent storage
   * Called during initialization
   */
  void loadSoundState();
  
  /**
   * Save sound state to persistent storage
   * Called when sound state changes
   */
  void saveSoundState();

  /**
   * Play a click sound for a detected radiation pulse
   * The frequency and duration can vary based on the radiation level
   */
  void playClick(uint32_t cps);

  /**
   * Toggle sound on/off
   * @return true if sound is now enabled, false if disabled
   */
  bool toggleSound();

  /**
   * Check if sound is currently enabled
   * @return true if sound is enabled, false otherwise
   */
  bool isSoundEnabled() const { return _sound_enabled; }

 private:
  bool _sound_enabled = true;
  uint32_t _last_cps = 0;
  uint32_t _last_click_time = 0;
  uint8_t _click_channel;
  
  // Constants for sound generation
  static constexpr uint16_t BASE_FREQUENCY = 1000;  // Base frequency in Hz
  static constexpr uint16_t MAX_FREQUENCY = 2500;  // Max frequency in Hz
  static constexpr uint16_t CLICK_DURATION = 10;   // Click duration in ms
  static constexpr uint16_t MIN_CLICK_INTERVAL = 200; // Minimum time between clicks in ms
};

#endif // BGEIGIEZEN_SOUND_MANAGER_H_
