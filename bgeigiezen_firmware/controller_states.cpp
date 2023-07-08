#include "controller_states.h"

// region InitializeHardwareState

InitializeHardwareState::InitializeHardwareState(Controller &context) : ControllerState(context)
{
}

void InitializeHardwareState::entry_action()
{
}

void InitializeHardwareState::do_activity()
{
}

void InitializeHardwareState::exit_action()
{
}

void InitializeHardwareState::handle_event(Event_enum event_id)
{
  switch (event_id)
  {
  default:
    ControllerState::handle_event(event_id);
    break;
  }
}

// endregion

// region InitializeSoftwareState

InitializeSoftwareState::InitializeSoftwareState(Controller &context) : ControllerState(context), timer(0)
{
}

void InitializeSoftwareState::entry_action()
{
}

void InitializeSoftwareState::do_activity()
{
}

void InitializeSoftwareState::exit_action()
{
}

void InitializeSoftwareState::handle_event(Event_enum event_id)
{
  switch (event_id)
  {
  default:
    ControllerState::handle_event(event_id);
    break;
  }
}

// endregion

// region ConfigModeState

ConfigurationModeState::ConfigurationModeState(Controller &context) : ControllerState(context)
{
}

void ConfigurationModeState::entry_action()
{
}

void ConfigurationModeState::do_activity()
{
}

void ConfigurationModeState::exit_action()
{
}

void ConfigurationModeState::handle_event(Event_enum event_id)
{
  switch (event_id)
  {
  default:
    ControllerState::handle_event(event_id);
    break;
  }
}

// endregion

// region MeasurementModeState

MeasurementModeState::MeasurementModeState(Controller &context) : ControllerState(context)
{
}

void MeasurementModeState::entry_action()
{
}

void MeasurementModeState::do_activity()
{
}

void MeasurementModeState::exit_action()
{
}

void MeasurementModeState::handle_event(Event_enum event_id)
{
  switch (event_id)
  {
  default:
    ControllerState::handle_event(event_id);
    break;
  }
}

// endregion

// region ResetState

ResetState::ResetState(Controller &context) : ControllerState(context)
{
}

void ResetState::entry_action()
{
}

void ResetState::do_activity()
{
  controller.reset_system();
}

void ResetState::exit_action()
{
}

void ResetState::handle_event(Event_enum event_id)
{
  ControllerState::handle_event(event_id);
}

// endregion
