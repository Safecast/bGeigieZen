#include <hardwarecounter.hpp>

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
  uint32_t *n_wraparound = (uint32_t *)arg;
  (*n_wraparound)++;
}

/* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 *  - set up the counter events to watch
 */
HardwareCounter::HardwareCounter(uint32_t delay, int gpio, pcnt_unit_t unit):
  _delay(delay), _gpio(gpio), _unit(unit)
{

  /* Prepare configuration for the PCNT unit */
  // Careful, in C++ the order of the fields should be the exact one of the definition
  pcnt_config_t pcnt_config = {
    // Set PCNT input signal and control GPIOs
    pulse_gpio_num : _gpio,
    ctrl_gpio_num : -1,  // no control pin is used
    // What to do when control input is low or high?
    lctrl_mode : PCNT_MODE_KEEP,  // Keep the primary counter mode if low
    hctrl_mode : PCNT_MODE_KEEP,  // Keep the primary counter mode if high
    // What to do on the positive / negative edge of pulse input?
    pos_mode : PCNT_COUNT_DIS,  // Keep the counter value on the positive edge
    neg_mode : PCNT_COUNT_INC,  // Count up on the negative edge
    // Set the maximum and minimum limit values to watch
    counter_h_lim : _max_value,  // max of 16 bit counter
    counter_l_lim : 0,

    unit : _unit,
    channel : PCNT_CHANNEL_0,
  };
  /* Initialize PCNT unit */
  pcnt_unit_config(&pcnt_config);

  /* Configure and enable the input filter */
  /*
     pcnt_set_filter_value(_unit, 100);  // we should set this to match the iRover
     pcnt_filter_enable(_unit);
     */

  /* Enable events on maximum limit value */
  pcnt_event_enable(_unit, PCNT_EVT_H_LIM);

  /* Initialize PCNT's counter */
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);

  /* Install interrupt service and add isr callback handler */
  pcnt_isr_service_install(0);
  pcnt_isr_handler_add(_unit, pcnt_example_intr_handler, (void *)&_n_wraparound);

  /* reset will timestamp and start the counter */
  reset();
}


void HardwareCounter::reset()
{
  pcnt_counter_pause(_unit);

  // set start time
  _start_time = millis();

  /* reset PCNT's counter */
  pcnt_counter_clear(_unit);
  _n_wraparound = 0;

  pcnt_counter_resume(_unit);
}

void HardwareCounter::update()
{
  // get current time
  unsigned long now = millis();

  // do basic check for millis overflow
  if ( (now >= _start_time && now - _start_time >= _delay)
      || (ULONG_MAX + now - _start_time >= _delay) ) {

    _last_count = _get_count_reset();
    _available = true;
  }
}
