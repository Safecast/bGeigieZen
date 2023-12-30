#include "log_aggregator.h"
#include "battery_indicator.h"
#include "debugger.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "identifiers.h"

#define D2R (PI / 180.0)
#define FIXED_LOCATION_RANGE_KM 0.4

/**
 * Convert degree degree to decimal minutes for log line
 * @param dd
 * @return dm
 */
double dd_to_dm(double dd) {
  double degrees = static_cast<int>(dd);
  double minutes = (dd - degrees) * 60;
  return (degrees) * 100 + minutes;
}



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

LogAggregator::LogAggregator(LocalStorage& settings) : ProcessWorker<DataLine>(5000), _settings(settings) {

}


int8_t LogAggregator::produce_data(const WorkerMap& workers) {
  const auto& gm_sensor_data = workers.worker<GeigerCounter>(k_worker_gm_sensor)->get_data();
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();

  data.cpm = gm_sensor_data.cpm_comp;
  data.latitude = gps_data.latitude;
  data.longitude = gps_data.longitude;
  data.altitude = gps_data.altitudeMSL;

  // Create log line (for logging and sending over bluetooth

  double latitude = 0;
  char NS = 'N';
  double longitude = 0;
  char WE = 'E';

  if (gps_data.valid()) {
    latitude = dd_to_dm(data.latitude < 0 ? data.latitude * -1 : data.latitude);
    NS = data.latitude < 0 ? 'S' : 'N';
    longitude = dd_to_dm(data.longitude < 0 ? data.longitude * -1 : data.longitude);
    WE = data.longitude < 0 ? 'W' : 'E';
    data.in_fixed_range = haversine_km(data.latitude, data.longitude, _settings.get_fixed_latitude(), _settings.get_fixed_longitude()) < FIXED_LOCATION_RANGE_KM;

    if ((_last_latitude < 0 || _last_latitude > 0) && (_last_longitude < 0 || _last_longitude > 0)) {
      double plus_distance = haversine_km(data.latitude, data.longitude, _last_latitude, _last_longitude);
      data.distance += plus_distance;
//      DEBUG_PRINTF("Distance %f = haversine_km(%f, %f, %f, %f)\n", plus_distance, data.latitude, data.longitude, _last_latitude, _last_longitude);
    }
    _last_latitude = data.latitude;
    _last_longitude = data.longitude;
  }

  sprintf(
      data.timestamp,
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      gps_data.year, gps_data.month, gps_data.day, gps_data.hour, gps_data.minute, gps_data.second);

  sprintf(
      data.log_string,
      "$%s,%04d,%s,%u,%u,%u,%c,%0.4f,%c,%0.4f,%c,%.1f,%c,%d,%d",
      DEVICE_HEADER, _settings.get_device_id(),
      data.timestamp,
      gm_sensor_data.cpm_comp, gm_sensor_data.cp5s, gm_sensor_data.total, gm_sensor_data.valid ? 'A' : 'V',
      latitude, NS, longitude, WE, data.altitude, gps_data.valid() ? 'A' : 'V', gps_data.satsInView,
      static_cast<int>(100 * gps_data.pdop));  // DOP logged as integer, displayed as float 

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

  return e_worker_data_read;
}
