#ifndef BGEIGIEZEN_API_DATA_CACHE_H_
#define BGEIGIEZEN_API_DATA_CACHE_H_

#include <Arduino.h>
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/battery_indicator.h"

/**
 * Singleton cache of the latest sensor readings, used by the HTTP
 * /api/v1/status endpoint.  Workers call update() each time they produce
 * new data; the web server reads it on demand.
 */
class ApiDataCache {
 public:
  static ApiDataCache& instance();

  void update(const GeigerData& g);
  void update(const GnssData& g);
  void update(const BatteryStatus& b);

  /**
   * Serialise the current snapshot to a JSON string.
   * @param device_id  Device ID from LocalStorage.
   * @param firmware   Firmware version string (e.g. "3.3.7").
   * @param mode       Operational mode name (e.g. "drive").
   */
  String toJson(uint16_t device_id, const char* firmware, const char* mode) const;

 private:
  ApiDataCache() = default;

  GeigerData    _geiger{};
  GnssData      _gnss{};
  BatteryStatus _battery{};
};

#endif // BGEIGIEZEN_API_DATA_CACHE_H_
