#include "message_timer.h"
#include "user_config.h"

// Initialize static members
unsigned long MessageTimer::clearTime = 0;
bool MessageTimer::messageScheduled = false;

void MessageTimer::init() {
  clearTime = 0;
  messageScheduled = false;
}

void MessageTimer::scheduleMessageClear() {
  // Set the time when the message should be cleared
  clearTime = millis() + STATUS_MESSAGE_DURATION;
  messageScheduled = true;
  
  // Debug output
  M5_LOGD("MessageTimer: Scheduled message clear at %lu ms", clearTime);
}

bool MessageTimer::checkAndClearMessage() {
  if (!messageScheduled) {
    return false;
  }
  
  // Check if it's time to clear the message
  if (millis() >= clearTime) {
    // Clear the message area
    M5_LOGD("MessageTimer: FORCE CLEARING message area at %lu ms", millis());
    M5.Lcd.fillRect(0, 220, 320, 20, TFT_BLACK); // Use TFT_BLACK for background color
    
    // Reset the timer
    messageScheduled = false;
    clearTime = 0;
    return true;
  }
  
  return false;
}
