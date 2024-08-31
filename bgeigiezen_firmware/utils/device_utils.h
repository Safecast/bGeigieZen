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
      // TODO: fix this for core2, it wont compile when enabled. ESP deep sleep doesnt work either.
//      M5.Power.powerOff();
//      ESP.deepSleep(0);
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
