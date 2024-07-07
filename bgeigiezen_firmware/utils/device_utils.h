#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif
#include "debugger.h"


class DeviceUtils {
 public:
  static void shutdown(bool reboot = false) {
    if (reboot) {
      ZEN_LOGD("\n Reboot system...\n\n");
      ESP.restart();
    } else {
      ZEN_LOGD("\n Shutdown system...\n\n");
#ifdef M5_CORE2
      M5.shutdown();
#elif M5_BASIC
      M5.Power.powerOFF();
#endif
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
