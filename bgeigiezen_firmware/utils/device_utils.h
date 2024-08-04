#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H

#include <M5Unified.hpp>
#include "debugger.h"


class DeviceUtils {
 public:
  static void shutdown(bool reboot = false) {
    if (reboot) {
      ZEN_LOGD("\n Reboot system...\n\n");
      ESP.restart();
    } else {
      ZEN_LOGD("\n Shutdown system...\n\n");
      M5.Power.powerOff();
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
