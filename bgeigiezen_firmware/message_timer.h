#ifndef BGEIGIEZEN_MESSAGE_TIMER_H_
#define BGEIGIEZEN_MESSAGE_TIMER_H_

#include <Arduino.h>
#include <M5Unified.hpp>
#include "user_config.h"

/**
 * A simple timer class to handle message timeouts
 * This is a standalone implementation that doesn't depend on other classes
 */
class MessageTimer {
public:
  // Initialize the timer
  static void init();
  
  // Schedule a message to be cleared after STATUS_MESSAGE_DURATION milliseconds
  static void scheduleMessageClear();
  
  // Check if it's time to clear the message and do so if needed
  static bool checkAndClearMessage();
  
private:
  static unsigned long clearTime;
  static bool messageScheduled;
};

#endif // BGEIGIEZEN_MESSAGE_TIMER_H_
