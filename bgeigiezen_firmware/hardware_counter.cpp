#include "hardware_counter.h"
#include "debugger.h"

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
void IRAM_ATTR pcnt_intr_handler(void* arg) {
  auto* n_wraparound = static_cast<uint32_t*>(arg);
  (*n_wraparound)++;
}

/* The timer interrupt routine
 * Records the number of pulses, reset the pulse counter, and set the
 * available flag
 */
void IRAM_ATTR timer_intr_handler(void* arg) {
  auto* counter = static_cast<HardwareCounter*>(arg);
  counter->_last_count = counter->_get_count_reset();
  counter->_available = true;
}

HardwareCounter::HardwareCounter() :
    _timer(),
    _delay_s(GEIGER_AVERAGING_PERIOD_S),
    _gpio(GEIGER_PULSE_GPIO),
    _unit(PCNT_UNIT_0),
    _max_value(std::numeric_limits<int16_t>::max()),
    _n_wraparound(0),
    _last_count(0),
    _available(false) {
}

bool HardwareCounter::available() const {
  return _available;
}

uint32_t HardwareCounter::get_last_count() {
  _available = false;  // remove flag after consumption
  return _last_count;
}

void HardwareCounter::begin() {
  /* Prepare configuration for the PCNT unit */
  // Careful, in C++ the order of the fields should be the exact one of the
  // definition
  pcnt_config_t pcnt_config = {
      // Set PCNT input signal and control GPIOs
      .pulse_gpio_num=_gpio,
      .ctrl_gpio_num=-1,  // no control pin is used
      // What to do when control input is low or high?
      .lctrl_mode=PCNT_MODE_KEEP,  // Keep the primary counter mode if low
      .hctrl_mode=PCNT_MODE_KEEP,  // Keep the primary counter mode if high
      // What to do on the positive / negative edge of pulse input?
      .pos_mode=PCNT_COUNT_DIS,  // Keep the counter value on the positive edge
      .neg_mode=PCNT_COUNT_INC,  // Count up on the negative edge
      // Set the maximum and minimum limit values to watch
      .counter_h_lim=_max_value,  // max of 16 bit counter
      .counter_l_lim=0,
      .unit=_unit,
      .channel=PCNT_CHANNEL_0,
  };
  /* Initialize PCNT unit */
  pcnt_unit_config(&pcnt_config);

  /* Configure and enable the input filter */
  /*
   pcnt_set_filter_value(_unit, 100);  // we should set this to match the
   iRover pcnt_filter_enable(_unit);
   */

  /* Enable events on maximum limit value */
  pcnt_event_enable(_unit, PCNT_EVT_H_LIM);

  /* Initialize PCNT's counter */
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);

  /* Install interrupt service and add isr callback handler */
  pcnt_isr_service_install(0);
  pcnt_isr_handler_add(_unit, pcnt_intr_handler, (void*) &_n_wraparound);

  // start the timer
  _timer.attach(static_cast<float>(_delay_s), timer_intr_handler, (void*) this);

  // reset and start counting
  reset();
}

void HardwareCounter::reset() {
  pcnt_counter_pause(_unit);

  // set start time
  _start_time = millis();
  _available = false;

  /* reset PCNT's counter */
  pcnt_counter_clear(_unit);
  _n_wraparound = 0;

  pcnt_counter_resume(_unit);
}

uint32_t HardwareCounter::_get_count_reset() {
  // compute current value of counter and reset
  int16_t count = 0;

  // get the value of the hardware counter
  esp_err_t ret = pcnt_get_counter_value(_unit, &count);
  if (ret != ESP_OK) {
    DEBUG_PRINTLN("A problem occurred in the hardware counter");
  }

  // compute the total value taking into account the wrap-arounds
  uint32_t total_count = _n_wraparound * _max_value + count;

  // reset the counter
  reset();

  return total_count;
}
