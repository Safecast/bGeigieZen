#ifndef BGEIGIEZEN_DEBUGGER_H_
#define BGEIGIEZEN_DEBUGGER_H_

#include <Arduino.h>
#include "user_config.h"

#ifdef UNIT_TEST
#undef ENABLE_DEBUG
#endif

#if ENABLE_DEBUG

#ifndef DEBUG_STREAM
#define DEBUG_STREAM Serial
#endif

#ifndef DEBUG_BAUD
#define DEBUG_BAUD 115200
#endif

#define DEBUG_BEGIN() DEBUG_STREAM.begin(DEBUG_BAUD)
#define DEBUG_PRINT(val) DEBUG_STREAM.print(val)
#define DEBUG_PRINTLN(val) DEBUG_STREAM.println(val)
#define DEBUG_PRINTF(format, ...) DEBUG_STREAM.printf(format, ##__VA_ARGS__)
#define DEBUG_FLUSH() DEBUG_STREAM.flush()

#else

#define DEBUG_BEGIN() NULL
#define DEBUG_PRINT(val) NULL
#define DEBUG_PRINTLN(val) NULL
#define DEBUG_PRINTF(format, ...) NULL
#define DEBUG_FLUSH() NULL

#endif

#endif // BGEIGIEZEN_DEBUGGER_H_
