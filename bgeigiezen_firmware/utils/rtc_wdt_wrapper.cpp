#include "rtc_wdt_wrapper.h"
#include <esp_system.h>
#include <esp_task_wdt.h>

#ifdef __cplusplus
extern "C" {
#endif

void rtc_wdt_wrapper_init(void) {
    // Initialize the RTC WDT if needed
    // This is a no-op for ESP32-S3 as the WDT is already initialized by default
}

void rtc_wdt_wrapper_set_stage_action(rtc_wdt_stage_action_t stage_sel, rtc_wdt_stage_action_t stage_act) {
    // Set the action for a specific WDT stage
    // This is a simplified version for ESP32-S3 compatibility
    (void)stage_sel;
    (void)stage_act;
}

void rtc_wdt_wrapper_set_time(uint32_t stage, unsigned int timeout_ms) {
    // Set the timeout for a specific WDT stage
    // This is a simplified version for ESP32-S3 compatibility
    (void)stage;
    (void)timeout_ms;
}

void rtc_wdt_wrapper_enable(void) {
    // Enable the RTC WDT
    // This is a simplified version for ESP32-S3 compatibility
    esp_task_wdt_init(5, true); // 5 seconds timeout, trigger panic
    esp_task_wdt_add(NULL); // Add current task to WDT
}

void rtc_wdt_wrapper_disable(void) {
    // Disable the RTC WDT
    // This is a simplified version for ESP32-S3 compatibility
    esp_task_wdt_delete(NULL); // Remove current task from WDT
}

void rtc_wdt_wrapper_feed(void) {
    // Feed the RTC WDT
    // This is a simplified version for ESP32-S3 compatibility
    esp_task_wdt_reset();
}

#ifdef __cplusplus
}
#endif
