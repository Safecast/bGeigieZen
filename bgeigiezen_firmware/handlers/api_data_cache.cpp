#include "api_data_cache.h"

#include "identifiers.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"

ApiDataCache& ApiDataCache::instance() {
  static ApiDataCache cache;
  return cache;
}

int8_t ApiDataCache::handle_produced_work(const worker_map_t& workers) {
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);

  if (!log_aggregator->is_fresh()) {
    return e_handler_idle;
  }

  const auto& gm_sensor_data = workers.worker<GeigerCounter>(k_worker_gm_sensor)->get_data();
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
  const auto& config = workers.worker<LocalStorage>(k_worker_local_storage);

  // Charging state as a string
  const char* charging_str = "unknown";
  switch (battery_data.isCharging) {
    case m5::Power_Class::is_charging_t::is_charging:
      charging_str = "charging";
      break;
    case m5::Power_Class::is_charging_t::is_discharging:
      charging_str = "discharging";
      break;
    case m5::Power_Class::is_charging_t::charge_unknown:
      charging_str = "unknown";
      break;
  }

  // Current mode as a string
  const char* mode_str = "unknown";
  switch (config->get_last_mode()) {
    case LocalStorage::e_operational_mode_satellite:
      mode_str = "satellite";
      break;
    case LocalStorage::e_operational_mode_flight:
      mode_str = "flight";
      break;
    case LocalStorage::e_operational_mode_drive:
      mode_str = "drive";
      break;
    case LocalStorage::e_operational_mode_survey:
      mode_str = "survey";
      break;
    case LocalStorage::e_operational_mode_fixed:
      mode_str = "real-time";
      break;
  }

  // Compose JSON using snprintf
  snprintf(_api_data_cache, sizeof(_api_data_cache),
    "{"
    "\"device\":{\"id\":%u,\"firmware\":\"%s\",\"mode\":\"%s\"},"
    "\"radiation\":{\"cpm\":%u,\"cpm_raw\":%u,\"uSvh\":%.4f,\"uSvh_5sec\":%.4f,\"Bqm2\":%.2f,\"valid\":%s,\"alert\":%s},"
    "\"gps\":{\"valid\":%s,\"latitude\":%.6f,\"longitude\":%.6f,\"altitude_m\":%.1f,\"speed_kmh\":%.2f,\"heading_deg\":%.1f,\"satellites_used\":%u,\"satellites_view\":%u,\"pdop\":%.1f},"
    "\"battery\":{\"pct\":%d,\"voltage_v\":%.2f,\"current_ma\":%.1f,\"charging\":\"%s\"},"
    "\"timestamp\":\"%s\",\"uptime_s\":%lu"
    "}",
    config->get_device_id(),
    VERSION_NUMBER,
    mode_str,
    gm_sensor_data.cpm_comp,
    gm_sensor_data.cpm_raw,
    gm_sensor_data.uSvh,
    gm_sensor_data.uSvh_5sec,
    gm_sensor_data.Bqm2,
    gm_sensor_data.valid ? "true" : "false",
    gm_sensor_data.alert ? "true" : "false",
    gps_data.valid() ? "true" : "false",
    gps_data.latitude,
    gps_data.longitude,
    static_cast<float>(gps_data.altitudeMSL),
    static_cast<float>(gps_data.gSpeed) * 0.0036f,
    static_cast<float>(gps_data.heading_degree),
    gps_data.numSV,
    gps_data.satsInView,
    static_cast<float>(gps_data.pdop),
    battery_data.percentage,
    battery_data.voltage,
    battery_data.current_mA,
    charging_str,
    gps_data.timestamp,
    static_cast<unsigned long>(millis() / 1000)
  );

  return e_handler_idle;
}

const char* ApiDataCache::get_latest_json() const {
  return _api_data_cache;
}
