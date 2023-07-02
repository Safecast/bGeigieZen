#ifndef STATE_MACHINE_EVENTS_H
#define STATE_MACHINE_EVENTS_H

typedef enum {
  e_undefined = -1,

  // Controller events
  e_c_button_pressed,
  e_c_button_long_pressed,
  e_c_controller_initialized,
  e_c_reading_initialized,
  e_c_post_init_time_passed,
  e_c_report_success,

} Event_enum;

#endif //STATE_MACHINE_EVENTS_H
