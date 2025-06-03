#include "ulp_geiger_counter.h"
#include <ulp.h> // duff2013/ulp library
#include <Arduino.h>

// ULP program: increments pulse_count in RTC memory on rising edge
const char ulp_prog[] = R"ulp(
    .global entry
    entry:
        // Read GPIO state
        READ_RTC_REG(0x3ff48400, 4, 1) // RTC_GPIO_IN_REG, bit for GPIO
        and r0, r0, 0x1 // Only care about LSB (GPIO state)
        move r1, 0
        move r2, 0
        ld r1, pulse_last_state, 0
        sub r2, r0, r1
        // If rising edge (r2 == 1), increment counter
        move r3, 1
        sub r4, r2, r3
        jump not_rising, r4, 1
        ld r5, pulse_count, 0
        add r5, r5, 1
        st r5, pulse_count, 0
    not_rising:
        st r0, pulse_last_state, 0
        halt
    .bss
    pulse_count: .long 0
    pulse_last_state: .long 0
)ulp";

// RTC memory variable offsets (in words)
#define ULP_PULSE_COUNT_OFFSET 0
#define ULP_PULSE_LAST_STATE_OFFSET 1

ULPGeigerCounter::ULPGeigerCounter() : _last_count(0), _rtc_gpio(-1) {}

bool ULPGeigerCounter::begin(int rtc_gpio_num) {
  _rtc_gpio = rtc_gpio_num;
  // Initialize RTC GPIO for input
  rtc_gpio_init((gpio_num_t)_rtc_gpio);
  rtc_gpio_set_direction((gpio_num_t)_rtc_gpio, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_en((gpio_num_t)_rtc_gpio);
  rtc_gpio_pullup_dis((gpio_num_t)_rtc_gpio);

  // Load and run the ULP program
  if (!ulp_load((const ulp_insn_t*)ulp_prog, sizeof(ulp_prog))) {
    Serial.println("ULP program load failed!");
    return false;
  }
  ulp_run(0);
  return true;
}

uint32_t ULPGeigerCounter::get_last_count() {
  // Read pulse count from ULP RTC memory
  uint32_t count = ulp_mem_read(ULP_PULSE_COUNT_OFFSET);
  _last_count = count;
  // Reset the counter in RTC memory
  ulp_mem_write(ULP_PULSE_COUNT_OFFSET, 0);
  return count;
}

void ULPGeigerCounter::reset() {
  ulp_mem_write(ULP_PULSE_COUNT_OFFSET, 0);
  _last_count = 0;
} 