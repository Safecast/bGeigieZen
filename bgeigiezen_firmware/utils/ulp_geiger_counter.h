#ifndef BGEIGIEZEN_ULP_GEIGER_COUNTER_H_
#define BGEIGIEZEN_ULP_GEIGER_COUNTER_H_

#include <stdint.h>

// ULPGeigerCounter: manages ULP-based pulse counting for Geiger tubes
class ULPGeigerCounter {
 public:
  ULPGeigerCounter();
  bool begin(int rtc_gpio_num); // Start ULP program on given RTC-capable GPIO
  uint32_t get_last_count();    // Get and reset the ULP pulse count
  void reset();                 // Reset the ULP counter

 private:
  uint32_t _last_count;
  int _rtc_gpio;
};

#endif // BGEIGIEZEN_ULP_GEIGER_COUNTER_H_ 