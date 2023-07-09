#include "controller_states.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "sd_wrapper.h"
#include "identifiers.h"

// region InitializeDeviceState

InitializeDeviceState::InitializeDeviceState(Controller& context) : ControllerState(context) {
}

void InitializeDeviceState::entry_action() {
}

void InitializeDeviceState::do_activity() {
  controller.initialize_device();
}

void InitializeDeviceState::exit_action() {
}

void InitializeDeviceState::handle_event(Event_enum event_id) {
  switch (event_id) {
    case Event_enum::e_c_device_initialized:
      controller.set_state(PostInitializeState::i(controller));
      break;
    case Event_enum::e_c_no_sd_card:
      controller.set_state(NoSDCardState::i(controller));
      break;
    case Event_enum::e_c_empty_sd_card:
      controller.set_state(EmptySDCardState::i(controller));
      break;
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region NoSDCardState

NoSDCardState::NoSDCardState(Controller& context) : ControllerState(context) {
}

void NoSDCardState::entry_action() {
}

void NoSDCardState::do_activity() {
}

void NoSDCardState::exit_action() {
}

void NoSDCardState::handle_event(Event_enum event_id) {
  switch (event_id) {
    case Event_enum::e_c_button_A_pressed:
    case Event_enum::e_c_button_B_pressed:
    case Event_enum::e_c_button_C_pressed:
      controller.shutdown(true);
      break;
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region EmptySDCardState

EmptySDCardState::EmptySDCardState(Controller& context) : ControllerState(context) {
}

void EmptySDCardState::entry_action() {
}

void EmptySDCardState::do_activity() {
}

void EmptySDCardState::exit_action() {
}

void EmptySDCardState::handle_event(Event_enum event_id) {
  switch (event_id) {
    case Event_enum::e_c_button_A_pressed:
    case Event_enum::e_c_button_B_pressed:
    case Event_enum::e_c_button_C_pressed:
      controller.handle_empty_sd();
      break;
    case Event_enum::e_c_sd_generated:
    case Event_enum::e_c_device_initialized:
      controller.set_state(PostInitializeState::i(controller));
      break;
    case Event_enum::e_c_no_sd_card:
//      controller.shutdown();
      // TODO: temp until config is implemented
      controller.set_state(PostInitializeState::i(controller));
      break;
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region PostInitializeState

PostInitializeState::PostInitializeState(Controller& context) : ControllerState(context), timer(0) {
}

void PostInitializeState::entry_action() {
  timer = millis();
}

void PostInitializeState::do_activity() {
  if (timer + 1000 < millis()) {
    controller.schedule_event(e_c_post_initialize_complete);
  }
}

void PostInitializeState::exit_action() {
}

void PostInitializeState::handle_event(Event_enum event_id) {
  switch (event_id) {
    case Event_enum::e_c_post_initialize_complete:
      controller.set_state(MeasurementModeState::i(controller));
      break;
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region ConfigModeState

ConfigurationModeState::ConfigurationModeState(Controller& context) : ControllerState(context) {
}

void ConfigurationModeState::entry_action() {
}

void ConfigurationModeState::do_activity() {
}

void ConfigurationModeState::exit_action() {
}

void ConfigurationModeState::handle_event(Event_enum event_id) {
  switch (event_id) {
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region MeasurementModeState

MeasurementModeState::MeasurementModeState(Controller& context) : ControllerState(context) {
}

void MeasurementModeState::entry_action() {
}

void MeasurementModeState::do_activity() {
}

void MeasurementModeState::exit_action() {
}

void MeasurementModeState::handle_event(Event_enum event_id) {
  switch (event_id) {
    default:
      ControllerState::handle_event(event_id);
      break;
  }
}

// endregion

// region ResetState

ResetState::ResetState(Controller& context) : ControllerState(context) {
}

void ResetState::entry_action() {
}

void ResetState::do_activity() {
  controller.reset_system();
}

void ResetState::exit_action() {
}

void ResetState::handle_event(Event_enum event_id) {
  ControllerState::handle_event(event_id);
}

// endregion
