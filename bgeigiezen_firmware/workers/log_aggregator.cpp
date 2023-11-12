#include "log_aggregator.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "battery_indicator.h"
#include "identifiers.h"

#define D2R (PI / 180.0)
#define FIXED_LOCATION_RANGE_KM 0.4


/* compute check sum of N bytes in array s */
char checksum(const char* s, size_t N) {
  int i = 0;
  char chk = s[i];
  for (i = 1; i < N; i++) {
    chk ^= s[i];
  };

  return chk;
}

/**
 * Calculate distance using haversine formula
 * @param lat1
 * @param long1
 * @param lat2
 * @param long2
 * @return distance in km
 */
double haversine_km(double lat1, double long1, double lat2, double long2) {
  double dlong = (long2 - long1) * D2R;
  double dlat = (lat2 - lat1) * D2R;
  double a = pow(sin(dlat / 2.0), 2) + cos(lat1 * D2R) * cos(lat2 * D2R) * pow(sin(dlong / 2.0), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return 6367 * c;
}

LogAggregator::LogAggregator(LocalStorage& settings) : Worker<DataLine>(5000), Handler(), _settings(settings) {}

int8_t LogAggregator::produce_data() {
  return 0;
}

int8_t LogAggregator::handle_produced_work(const WorkerMap& workers) {
  const auto& gm_sensor_data = workers.worker<GeigerCounter>(k_worker_gm_sensor)->get_data();
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();

  if (gps_data.valid()) {
    data.in_fixed_range = haversine_km(gps_data.latitude, gps_data.longitude, _settings.get_fixed_latitude(), _settings.get_fixed_longitude()) < FIXED_LOCATION_RANGE_KM;
  }
  data.cpm = gm_sensor_data.cpm_comp;
  data.latitude = gps_data.latitude;
  data.longitude = gps_data.longitude;
  data.altitude = gps_data.altitudeMSL;

  // Create log line (for logging and sending over bluetooth

  double latitude = data.latitude < 0 ? data.latitude * -1 : data.latitude;
  char NS = data.latitude < 0 ? 'S' : 'N';
  double longitude = data.longitude < 0 ? data.longitude * -1 : data.longitude;
  char WE = data.longitude < 0 ? 'W' : 'E';

  sprintf(
      data.timestamp,
      "%02d-%02d-%02dT%02d:%02d:%02dZ",
      gps_data.year, gps_data.month, gps_data.day, gps_data.hour, gps_data.minute, gps_data.second);

  sprintf(
      data.log_string,
      "$%s,%04d,%s,%uld,%uld,%uld,%c,%0.7f,%c,%0.7f,%c,%.1f,%c,%d,%.1f",
      DEVICE_HEADER, _settings.get_device_id(),
      data.timestamp,
      gm_sensor_data.cpm_comp, gm_sensor_data.cp5s, gm_sensor_data.total, gm_sensor_data.valid ? 'A' : 'V',
      latitude, NS, longitude, WE, data.altitude, gps_data.valid() ? 'A' : 'V', gps_data.satsInView, gps_data.hdop);

  size_t len = 0;
  len = strlen(data.log_string);
  data.log_string[len] = '\0';

  // generate checksum
  uint8_t chk = checksum(data.log_string + 1, len);

  // add checksum to end of line before sending
  if (chk < 16)
    sprintf(data.log_string + len, "*0%X", (unsigned int)chk);
  else
    sprintf(data.log_string + len, "*%X", (unsigned int)chk);

  data.valid = gps_data.valid() && gm_sensor_data.valid;

  return e_handler_data_handled;
}
