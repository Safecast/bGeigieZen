#ifndef __FSM_STATE_BASE_HPP
#define __FSM_STATE_BASE_HPP

#include <fsm_context.hpp>

// preemptive declaration
class Context;

namespace bGeigieZen {
  enum class Event {
    DEVICE_STARTED,
    SETUP_FINISHED,
    GPS_DATE_BECAME_CORRECT,
    LOG_FILE_CREATED,
    GPS_GEIGER_COUNT_READY,
  };
}

class State {
  protected:
    Context *_ctx = NULL;
  public:
    State() {}
    void enter(Context *ctx, bGeigieZen::Event e) {
      _ctx = ctx;
      on_enter(e);
    }
    void exit(bGeigieZen::Event e) {
      on_exit(e);
      _ctx = NULL;
    }
    virtual void on_enter(bGeigieZen::Event e) = 0;
    virtual void process() = 0;
    virtual void on_exit(bGeigieZen::Event e) = 0;
};

#endif // __FSM_STATE_BASE_HPP
