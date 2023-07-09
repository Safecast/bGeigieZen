#ifndef BGEIGIEZEN_SM_C_CONCRETE_STATES_H
#define BGEIGIEZEN_SM_C_CONCRETE_STATES_H

#include "controller.h"

/**
 * AbstractState with controller context, so the states can control the system
 * Must be inherited with singleton implementation
 */
class ControllerState : public AbstractState {
 public:
  ControllerState(const ControllerState&) = delete;
  ControllerState& operator=(const ControllerState&) = delete;
  ControllerState(ControllerState&&) = delete;
  ControllerState& operator=(ControllerState&&) = delete;

 protected:
  explicit ControllerState(Controller& context) : controller(context) {};

  Controller& controller;
};

/**
 * Initialize device, M5 components, config and sd card
 */
class InitializeDeviceState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static InitializeDeviceState& i(Controller& context) {
    static InitializeDeviceState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_InitializeDeviceState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit InitializeDeviceState(Controller& context);
};

/**
 * No SD card found on initialization
 */
class NoSDCardState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static NoSDCardState& i(Controller& context) {
    static NoSDCardState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_NoSDCardState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit NoSDCardState(Controller& context);
};

/**
 * Empty SD card found on initialization
 */
class EmptySDCardState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static EmptySDCardState& i(Controller& context) {
    static EmptySDCardState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_EmptySDCardState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit EmptySDCardState(Controller& context);
};

class PostInitializeState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static PostInitializeState& i(Controller& context) {
    static PostInitializeState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_PostInitializeState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit PostInitializeState(Controller& context);

 private:
  uint32_t timer;
};

class ConfigurationModeState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static ConfigurationModeState& i(Controller& context) {
    static ConfigurationModeState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_ConfigurationModeState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit ConfigurationModeState(Controller& context);

};

class MeasurementModeState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static MeasurementModeState& i(Controller& context) {
    static MeasurementModeState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_MeasurementModeState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit MeasurementModeState(Controller& context);
};

class ResetState : public ControllerState {
 public:
  /**
   * Singleton
   */
  static ResetState& i(Controller& context) {
    static ResetState state(context);
    return state;
  }

  int8_t get_state_id() const override { return k_state_ResetState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;

  void handle_event(Event_enum event_id) override;

 protected:
  explicit ResetState(Controller& context);
};

#endif // BGEIGIEZEN_SM_C_CONCRETE_STATES_H
