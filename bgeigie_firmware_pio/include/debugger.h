#ifndef BGEIGIECAST_DEBUGGER_H
#define BGEIGIECAST_DEBUGGER_H

#include "user_config.h"

#ifdef UNIT_TEST
#undef ENABLE_DEBUG
#endif

#if ENABLE_DEBUG
#include <Arduino.h>

#ifndef DEBUG_STREAM
#define DEBUG_STREAM Serial
#endif

#define DEBUG_BEGIN(baud) DEBUG_STREAM.begin(baud)
#define DEBUG_PRINT(val) DEBUG_STREAM.print(val)
#define DEBUG_PRINTLN(val) DEBUG_STREAM.println(val)
#define DEBUG_PRINTF(format, ...) DEBUG_STREAM.printf(format,  ##__VA_ARGS__)
#define DEBUG_FLUSH() DEBUG_STREAM.flush()

#else

#define DEBUG_BEGIN(...)
#define DEBUG_PRINT(val)
#define DEBUG_PRINTLN(val)
#define DEBUG_PRINTF(format, ...)
#define DEBUG_FLUSH()

#endif

#endif //BGEIGIECAST_DEBUGGER_H
