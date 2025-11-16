#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H

#include <M5Unified.hpp>
<<<<<<< HEAD
#include "workers/gps_connector.h"

// Expose global pointer to active GPS connector (defined in main.cpp)
extern GpsConnector* g_active_gps;
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416


class DeviceUtils {
 public:
  static void shutdown(bool reboot = false) {
    if (reboot) {
<<<<<<< HEAD
      M5_LOGI("System reboot initiated");
      M5_LOGD("\n Reboot system...\n");
      ESP.restart();
    } else {
      M5_LOGI("Power down initiated - saving GPS data");
      if (g_active_gps) {
        M5_LOGI("GPS: Backing up satellite data and memory to internal storage");
        g_active_gps->backupGpsMemoryToNVS();
        delay(50); // give UART time to send
        M5_LOGI("GPS: Satellite data backup completed");
      }
      M5_LOGI("System shutdown in progress");
=======
      M5_LOGD("\n Reboot system...\n");
      ESP.restart();
    } else {
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
      M5_LOGD("\n Shutdown system...\n");
      // TODO: fix this for core2, it wont compile when enabled. ESP deep sleep doesnt work either.
//      M5.Power.powerOff();
//      ESP.deepSleep(0);
    }
  }
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_DEVICE_UTILS_H
