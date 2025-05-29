#ifndef RTC_WDT_WRAPPER_H
#define RTC_WDT_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTC_WDT_STAGE_ACTION_OFF = 0,           /*!< Disabled. This stage will have no effects on the system. */
    RTC_WDT_STAGE_ACTION_INTERRUPT = 1,     /*!< Trigger an interrupt. When the stage expires an interrupt is triggered. */
    RTC_WDT_STAGE_ACTION_RESET_CPU = 2,     /*!< Reset a CPU core. */
    RTC_WDT_STAGE_ACTION_RESET_SYSTEM = 3,  /*!< Reset the main system includes the CPU and all peripherals. The RTC is an exception to this, and it will not be reset. */
    RTC_WDT_STAGE_ACTION_RESET_RTC = 4      /*!< Reset the main system and the RTC. */
} rtc_wdt_stage_action_t;

void rtc_wdt_wrapper_init(void);
void rtc_wdt_wrapper_set_stage_action(rtc_wdt_stage_action_t stage_sel, rtc_wdt_stage_action_t stage_act);
void rtc_wdt_wrapper_set_time(uint32_t stage, unsigned int timeout_ms);
void rtc_wdt_wrapper_enable(void);
void rtc_wdt_wrapper_disable(void);
void rtc_wdt_wrapper_feed(void);

#ifdef __cplusplus
}
#endif

#endif // RTC_WDT_WRAPPER_H
