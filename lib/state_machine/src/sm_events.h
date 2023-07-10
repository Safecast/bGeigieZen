#ifndef STATE_MACHINE_EVENTS_H
#define STATE_MACHINE_EVENTS_H

typedef enum {
  e_undefined = -1,

  // Controller events
  e_c_device_initialized,
  e_c_no_sd_card,
  e_c_empty_sd_card,
  e_c_post_initialize_complete,
  e_c_button_A_pressed,
  e_c_button_A_long_pressed,
  e_c_button_B_pressed,
  e_c_button_B_long_pressed,
  e_c_button_C_pressed,
  e_c_button_C_long_pressed,
  e_c_controller_initialized,
  e_c_sd_generated,
  e_c_reading_initialized,
  e_c_post_init_time_passed,
  e_c_report_success,

} Event_enum;

#endif //STATE_MACHINE_EVENTS_H
