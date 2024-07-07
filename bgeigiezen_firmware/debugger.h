#ifndef BGEIGIEZEN_DEBUGGER_H_
#define BGEIGIEZEN_DEBUGGER_H_

#include <Arduino.h>
#include "user_config.h"


#ifdef UNIT_TEST
#undef ENABLE_DEBUG
#endif


/**
   ESP_LOGE - error (lowest)

   ESP_LOGW - warning

   ESP_LOGI - info

   ESP_LOGD - debug

   ESP_LOGV - verbose (highest)
 */

/// Output log with source info.
#define ZEN_LOG_FORMAT(letter, format) "[%6u][" #letter "]: " format, millis()

/// Output Error log with source info.
#define ZEN_LOGE(format, ...) ESP_LOGE(ESP_LOG_ERROR  , ZEN_LOG_FORMAT("E", format), ##__VA_ARGS__)

/// Output Warn log with source info.
#define ZEN_LOGW(format, ...) ESP_LOGW(ESP_LOG_WARN   , ZEN_LOG_FORMAT("W", format), ##__VA_ARGS__)

/// Output Info log with source info.
#define ZEN_LOGI(format, ...) ESP_LOGI(ESP_LOG_INFO   , ZEN_LOG_FORMAT("I", format), ##__VA_ARGS__)

/// Output Debug log with source info.
#define ZEN_LOGD(format, ...) ESP_LOGD(ESP_LOG_DEBUG  , ZEN_LOG_FORMAT("D", format), ##__VA_ARGS__)

/// Output Verbose log with source info.
#define ZEN_LOGV(format, ...) ESP_LOGV(ESP_LOG_VERBOSE, ZEN_LOG_FORMAT("V", format), ##__VA_ARGS__)

#endif // BGEIGIEZEN_DEBUGGER_H_
