#include "sound_manager.h"
#include "identifiers.h"
#include "gm_sensor.h"
#include "controller.h"

SoundManager::SoundManager() :
    Worker("SoundManager"),
    _sound_enabled(true),
    _last_cps(0),
    _last_click_time(0),
    _click_channel(0) {
  // Load sound state from preferences
  loadSoundState();
  
  // Set data to match loaded sound state
  data = _sound_enabled;
  
  // Set speaker volume
  M5.Speaker.setVolume(200);
}

bool SoundManager::activate(bool retry) {
  // Load sound state from preferences
  loadSoundState();
  
  // Update the worker data to match the loaded sound state
  data = _sound_enabled;
  
  M5_LOGD("Sound manager activated, sound is %s", _sound_enabled ? "ON" : "OFF");
  
  return true;
}

int8_t SoundManager::produce_data() {
  // This method is called regularly by the controller
  
  // If sound is enabled, simulate clicks based on radiation level
  if (_sound_enabled) {
    uint32_t current_time = millis();
    
    // We'll use a probabilistic approach to simulate clicks based on CPS
    // This will make clicks more random like actual Geiger counter clicks
    
    // The higher the CPS, the more likely we are to play a click in any given interval
    // For example, if CPS is 10, we should play approximately 10 clicks per second
    
    // Calculate probability of a click in this interval
    // We check every ~10ms, so for a CPS of 10, probability should be 10/100 = 0.1
    float click_probability = _last_cps / 100.0f;
    
    // Generate a random number between 0 and 1
    float random_value = (float)random(100) / 100.0f;
    
    // If random value is less than probability and enough time has passed, play a click
    if (random_value < click_probability && current_time - _last_click_time >= MIN_CLICK_INTERVAL) {
      // Play a click sound with a higher volume
      M5.Speaker.setVolume(200);
      
      // Vary frequency based on CPS - higher CPS = higher frequency
      uint16_t frequency = BASE_FREQUENCY + (_last_cps * 10);
      if (frequency > MAX_FREQUENCY) frequency = MAX_FREQUENCY;
      
      M5.Speaker.tone(frequency, CLICK_DURATION, _click_channel, true);
      
      // Update last click time
      _last_click_time = current_time;
      
      // Rotate through channels to allow overlapping clicks
      _click_channel = (_click_channel + 1) % 8;
    }
    
    // Every second, update the CPS value from the global data
    static uint32_t last_cps_update = 0;
    if (current_time - last_cps_update >= 1000) {
      // Get CPS from global data (this is a hack, but should work)
      extern uint16_t g_cps;
      _last_cps = g_cps;
      last_cps_update = current_time;
    }
  }
  
  // Update our data (sound enabled status)
  data = _sound_enabled;
  return e_worker_data_read;
}

void SoundManager::playClick(uint32_t cps) {
  if (!_sound_enabled) {
    return;
  }

  uint32_t current_time = millis();
  
  // Limit the click rate to avoid overwhelming the speaker
  if (current_time - _last_click_time < MIN_CLICK_INTERVAL) {
    return;
  }
  
  // Calculate frequency based on CPS
  // Higher CPS = higher frequency
  uint16_t frequency = BASE_FREQUENCY + (cps * 10);
  if (frequency > MAX_FREQUENCY) frequency = MAX_FREQUENCY;
  
  // Play the click sound
  M5.Speaker.tone(frequency, CLICK_DURATION, _click_channel);
  
  // Update last click time and CPS
  _last_click_time = millis();
  _last_cps = cps;
}

void SoundManager::loadSoundState() {
  // Use the same namespace as other settings
  Preferences preferences;
  
  // Open the preferences namespace in read-only mode
  if (preferences.begin("bgeigiezen", true)) {
    // Load sound enabled state (default to true if not found)
    _sound_enabled = preferences.getBool("sound_enabled", true);
    
    // Close the preferences
    preferences.end();
    
    M5_LOGD("Loaded sound state: %s", _sound_enabled ? "ON" : "OFF");
  } else {
    // If we can't open preferences, default to enabled
    _sound_enabled = true;
    M5_LOGD("Failed to load sound state, defaulting to ON");
  }
}

void SoundManager::saveSoundState() {
  // Use the same namespace as other settings
  Preferences preferences;
  
  // Open the preferences namespace in read-write mode
  if (preferences.begin("bgeigiezen", false)) {
    // Save sound enabled state
    preferences.putBool("sound_enabled", _sound_enabled);
    
    // Close the preferences
    preferences.end();
    
    M5_LOGD("Saved sound state: %s", _sound_enabled ? "ON" : "OFF");
  } else {
    M5_LOGD("Unable to save sound state");
  }
}

bool SoundManager::toggleSound() {
  // Log the sound toggle operation
  M5_LOGD("Toggling sound state from %s", _sound_enabled ? "ON" : "OFF");
  
  // Toggle sound state
  _sound_enabled = !_sound_enabled;
  data = _sound_enabled;
  
  // Save the new sound state to preferences
  saveSoundState();
  
  // Force maximum volume for confirmation tones
  M5.Speaker.setVolume(255);
  
  // Play a confirmation tone
  if (_sound_enabled) {
    // Sound enabled - play ascending tones
    M5_LOGD("Playing enabled confirmation tones");
    M5.Speaker.tone(440, 150, 0, true);
    delay(150);
    M5.Speaker.tone(880, 150, 0, true);
    delay(150);
    M5.Speaker.tone(1320, 150, 0, true);
    delay(150);
  } else {
    // Sound disabled - play descending tones
    M5_LOGD("Playing disabled confirmation tones");
    M5.Speaker.tone(1320, 150, 0, true);
    delay(150);
    M5.Speaker.tone(880, 150, 0, true);
    delay(150);
    M5.Speaker.tone(440, 150, 0, true);
    delay(150);
  }
  
  // Reset to normal volume
  M5.Speaker.setVolume(200);
  
  M5_LOGD("Sound toggled to %s", _sound_enabled ? "ON" : "OFF");
  
  return _sound_enabled;
}

void SoundManager::playErrorBeeps() {
  // Save current volume
  uint8_t current_volume = M5.Speaker.getVolume();
  
  // Set maximum volume for error beeps (critical alerts)
  M5.Speaker.setVolume(255);
  
  M5_LOGD("Playing error beeps (3x 3kHz)");
  
  // Play three 3kHz beeps with short pauses
  for (int i = 0; i < 3; i++) {
    M5.Speaker.tone(3000, 200, 0, true);  // 3kHz for 200ms
    delay(200);  // Wait for tone to complete
    if (i < 2) {  // Don't delay after the last beep
      delay(100);  // 100ms pause between beeps
    }
  }
  
  // Restore original volume
  M5.Speaker.setVolume(current_volume);
}

void SoundManager::playCpmAlert() {
  // Only play if sound is enabled (unlike error beeps which always play)
  if (!_sound_enabled) {
    return;
  }
  
  // Save current volume
  uint8_t current_volume = M5.Speaker.getVolume();
  
  // Set high volume for alert (but not max like error beeps)
  M5.Speaker.setVolume(220);
  
  M5_LOGD("Playing CPM alert sound");
  
  // Play a distinct alert pattern: two quick high-pitched beeps
  // Different from error beeps (3x 3kHz) and regular clicks (variable frequency)
  M5.Speaker.tone(2500, 150, 0, true);  // 2.5kHz for 150ms
  delay(150);
  delay(50);   // Short pause
  M5.Speaker.tone(2500, 150, 0, true);  // Second beep
  delay(150);
  
  // Restore original volume
  M5.Speaker.setVolume(current_volume);
}
