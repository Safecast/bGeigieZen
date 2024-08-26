#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H

#include <M5Unified.hpp>


class DeviceUtils {
 public:
  static void shutdown(bool reboot = false) {
    if (reboot) {
      M5_LOGD("\n Reboot system...\n");
      ESP.restart();
    } else {
      M5_LOGD("\n Shutdown system...\n");
      M5.Power.powerOff();
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
