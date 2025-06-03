#include <Arduino.h>
// #include "UlpDebug.h"
#include "driver/rtc_io.h"
#include <soc/soc_ulp.h>
#include "ulp_main.h"
// #include "ulptool.h"

// RTC slow memory variable indices
enum {
  SLOW_PULSE_COUNT,   // Pulse count
  SLOW_LAST_STATE,    // Last pin state
  SLOW_PROG_ADDR
};

// Unlike the esp-idf always use these binary blob names
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

void setup() {
  Serial0.begin(115200);
  delay(1000);

  // RTC GPIO32 setup (input only)
  rtc_gpio_init(GPIO_NUM_32);
  rtc_gpio_set_direction(GPIO_NUM_32, RTC_GPIO_MODE_INPUT_ONLY);

  // Set ULP wakeup period (e.g. 10ms)
  ulp_set_wakeup_period(0, 10000);

  // Clear RTC slow memory
  memset(RTC_SLOW_MEM, 0, 8192);

  // Load and start ULP program using ulptool_load_binary
  // esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
  // if (err != ESP_OK) {
  //     Serial0.printf("ULP binary load failed: %d\n", err);
  //     return;
  // }

  // Use the symbol provided by the generated header for the entry point
  // err = ulp_run(((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
  //   if (err != ESP_OK) {
  //     Serial0.printf("ULP run failed: %d\n", err);
  //     return;
  // }

  Serial0.println("ULP Geiger pulse counter started on GPIO32 (CoreS3)");
}

unsigned long last_cpm_time = 0;
uint32_t last_pulse_count = 0;

void loop() {
  // Print ULP program disassembly for debugging (optional)
  // ulpDump();

  // Read pulse count from RTC slow memory
  uint32_t pulse_count = RTC_SLOW_MEM[SLOW_PULSE_COUNT];

  // Calculate CPM every 10 seconds
  unsigned long now = millis();
  if (now - last_cpm_time >= 10000) {
    uint32_t delta = pulse_count - last_pulse_count;
    float cpm = (delta * 60.0f) / 10.0f; // 10s window
    Serial0.print("Pulse count: ");
    Serial0.print(pulse_count);
    Serial0.print(" | CPM: ");
    Serial0.println(cpm, 1);

    last_pulse_count = pulse_count;
    last_cpm_time = now;
  }

  delay(100);
} 