#ifndef STATE_MACHINE_ABSTRACT_STATE_HPP
#define STATE_MACHINE_ABSTRACT_STATE_HPP

#include <stdint.h>
#include "sm_events.h"

/**
 * Abstract state for the state machine pattern
 */
class AbstractState {
 public:
  AbstractState() = default;;
  virtual ~AbstractState() = default;

  /**
   * Get some unique identifier of the state
   * @return
   */
  virtual int8_t get_state_id() const = 0;

  /**
   * Action when entering this state
   */
  virtual void entry_action() = 0;

  /**
   * Activity while in this state, will be called constantly through the main loop
   */
  virtual void do_activity() = 0;

  /**
   * Action when exiting the state
   */
  virtual void exit_action() = 0;

  /**
   * Handle event
   * @param event_id
   */
  virtual void handle_event(Event_enum event_id) {
  }
};

#endif //STATE_MACHINE_STATE_HPP
