#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H

#include <M5Unified.hpp>
#include "workers/gps_connector.h"

// Expose global pointer to active GPS connector (defined in main.cpp)
extern GpsConnector* g_active_gps;


class DeviceUtils {
 public:
  static void shutdown(bool reboot = false) {
    if (reboot) {
      M5_LOGD("\n Reboot system...\n");
      ESP.restart();
    } else {
      if (g_active_gps) {
        g_active_gps->requestBackup();
        delay(50); // give UART time to send
      }
      M5_LOGD("\n Shutdown system...\n");
      // TODO: fix this for core2, it wont compile when enabled. ESP deep sleep doesnt work either.
//      M5.Power.powerOff();
//      ESP.deepSleep(0);
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
