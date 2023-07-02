#ifndef __FSM_STATE_CONCRETE_HPP__
#define __FSM_STATE_CONCRETE_HPP__

#include <fsm_state_base.hpp>

class StateStartup : public State {
 private:
  static StateStartup *_instance;

 protected:
  StateStartup() {}

 public:
  void on_enter(bGeigieZen::Event e);
  void process();
  void on_exit(bGeigieZen::Event e);
  static StateStartup *instance();
};

class StateWaitGPSTime : public State {
 private:
  static StateWaitGPSTime *_instance;

 protected:
  StateWaitGPSTime() {}

 public:
  void on_enter(bGeigieZen::Event e);
  void process();
  void on_exit(bGeigieZen::Event e);
  static StateWaitGPSTime *instance();
};

class StateLogging : public State {
 private:
  static StateLogging *_instance;

 protected:
  StateLogging() {}

 public:
  void on_enter(bGeigieZen::Event e);
  void process();
  void on_exit(bGeigieZen::Event e);
  static StateLogging *instance();
};
#endif  // __FSM_STATE_CONCRETE_HPP__
