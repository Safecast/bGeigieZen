#ifndef __FSM_HPP__
#define __FSM_HPP__
/* Defines all the states of the device */

#include <M5Stack.h>

enum bg_state_t {
  BG_STARTUP,
  BG_GPS_TIME_NOT_ACQUIRED,
  BG_CREATE_LOG_FILE,
  BG_LOGGING,
};

enum bg_event_t {
  EVENT_SETUP_FINISHED,
  EVENT_GPS_DATE_BECAME_CORRECT,
  EVENT_LOG_FILE_CREATED,
  EVENT_GPS_GEIGER_COUNT_READY,
};

class FiniteStateMachine {
 private:
  bg_state_t _state = BG_STARTUP;

 public:
  bg_state_t get_state() { return _state; }
  void move_to(bg_state_t new_state) { _state = new_state; }
  void register_event(bg_event_t new_event) {
    switch (_state) {
      case BG_STARTUP:
        switch (new_event) {
          case EVENT_SETUP_FINISHED:
            _state = BG_GPS_TIME_NOT_ACQUIRED;
            break;
          default:
            Serial.println("Invalid state/event combination");
            break;
        }
        break;

      case BG_GPS_TIME_NOT_ACQUIRED:
        switch (new_event) {
          case EVENT_GPS_DATE_BECAME_CORRECT:
            _state = BG_CREATE_LOG_FILE;
            break;
          default:
            Serial.println("Invalid state/event combination");
            break;
        }
        break;

      case BG_CREATE_LOG_FILE:
        switch (new_event) {
          case EVENT_LOG_FILE_CREATED:
            _state = BG_LOGGING;
            break;
          default:
            Serial.println("Invalid state/event combination");
            break;
        }

      case BG_LOGGING:
        switch (new_event) {
          default:
            Serial.println("Invalid state/event combination");
            break;
        }
        break;
    }
  }
};

#endif  // __FSM_HPP__
