#ifndef BGEIGIEZEN_SM_C_CONCRETE_STATES_H
#define BGEIGIEZEN_SM_C_CONCRETE_STATES_H

#include "controller.h"

/**
 * AbstractState with controller context, so the states can control the system
 */
class ControllerState : public AbstractState {
 public:

  explicit ControllerState(Controller& context) : controller(context) {};
  virtual ~ControllerState() = default;

 protected:
  Controller& controller;
};


class InitializeHardwareState : public ControllerState {
 public:
  explicit InitializeHardwareState(Controller& context);
  virtual ~InitializeHardwareState() = default;

  int8_t get_state_id() const override { return k_state_InitializeHardwareState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;
  void handle_event(Event_enum event_id) override;
};

class InitializeSoftwareState : public ControllerState {
 public:
  explicit InitializeSoftwareState(Controller& context);
  virtual ~InitializeSoftwareState() = default;

  int8_t get_state_id() const override { return k_state_InitializeSoftwareState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;
  void handle_event(Event_enum event_id) override;

 private:
  uint32_t timer;
};

class ConfigurationModeState : public ControllerState {
 public:
  explicit ConfigurationModeState(Controller& context);
  virtual ~ConfigurationModeState() = default;

  int8_t get_state_id() const override { return k_state_ConfigurationModeState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;
  void handle_event(Event_enum event_id) override;
};

class MeasurementModeState : public ControllerState {
 public:
  explicit MeasurementModeState(Controller& context);
  virtual ~MeasurementModeState() = default;

  int8_t get_state_id() const override { return k_state_MeasurementModeState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;
  void handle_event(Event_enum event_id) override;

};

class ResetState : public ControllerState {
 public:
  explicit ResetState(Controller& context);
  virtual ~ResetState() = default;

  int8_t get_state_id() const override { return k_state_ResetState; }

  void entry_action() override;
  void do_activity() override;
  void exit_action() override;
  void handle_event(Event_enum event_id) override;
};

#endif //BGEIGIEZEN_SM_C_CONCRETE_STATES_H
