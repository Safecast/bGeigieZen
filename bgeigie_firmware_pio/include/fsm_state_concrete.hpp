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

  // EventHandlerCallback invoke_menu(Event& e);
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

  // void invoke_menu(Event& e);
};

class StateMenu : public State {
 private:
  static StateMenu *_instance;

 protected:
  StateMenu() {}

 public:
  void on_enter(bGeigieZen::Event e);
  void process();
  void on_exit(bGeigieZen::Event e);
  static StateMenu *instance();
};

class StateConfig : public State {
 private:
  static StateConfig *_instance;

 protected:
  StateConfig() {}

 public:
  void on_enter(bGeigieZen::Event e);
  void process();
  void on_exit(bGeigieZen::Event e);
  static StateConfig *instance();
};

#endif  // __FSM_STATE_CONCRETE_HPP__
