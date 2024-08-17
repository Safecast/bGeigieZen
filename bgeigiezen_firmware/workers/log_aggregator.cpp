#include "log_aggregator.h"
#include "battery_indicator.h"
#include "debugger.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "pwrmon_connector.h"
#include "identifiers.h"

#define D2R (PI / 180.0)
#define FIXED_LOCATION_RANGE_KM 1.0
#define LOG_SECOND_TO_TIMEOUT(sec) ((sec * 1000) - 100)

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

/*  */
/**
 * Compute check sum of N bytes in array s
 * @param s:
 * @param N: size of s
 * @return checksum
 */
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

LogAggregator::LogAggregator(LocalStorage& settings) : ProcessWorker<DataLine>(LOG_SECOND_TO_TIMEOUT(LOG_SECONDS_DELAY)), _settings(settings) {

}


int8_t LogAggregator::produce_data(const WorkerMap& workers) {
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);

  if (!gm_sensor->is_fresh()) {
    // No fresh data yet, log only once every 5 seconds
    return e_worker_idle;
  }

  const auto& gm_sensor_data = gm_sensor->get_data();
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  const auto& battery_data = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
  const auto& pwrmon_data = workers.worker<PwrmonConnector>(k_worker_pwrmon_connector)->get_data();

  data.cpm = gm_sensor_data.cpm_comp;
  data.latitude = gps_data.latitude;
  data.longitude = gps_data.longitude;
  data.altitude = gps_data.altitudeMSL;

  // Create log line (for logging and sending over bluetooth
  uint16_t latitude_dm = 0;
  uint16_t latitude_s = 0;
  char NS = 'N';
  uint16_t longitude_dm = 0;
  uint16_t longitude_s = 0;
  char WE = 'E';

  if (gps_data.valid()) {
    if (_settings.get_fixed_latitude() != 0 && _settings.get_fixed_longitude() != 0 && _settings.get_fixed_range() > 0) {
      data.in_fixed_range = haversine_km(data.latitude, data.longitude, _settings.get_fixed_latitude(), _settings.get_fixed_longitude()) < _settings.get_fixed_range();
    } else {
      data.in_fixed_range = false;
    }

    if ((_last_latitude < 0 || _last_latitude > 0) && (_last_longitude < 0 || _last_longitude > 0)) {
      double plus_distance = haversine_km(data.latitude, data.longitude, _last_latitude, _last_longitude);
      data.distance += plus_distance;
      //      DEBUG_PRINTF("Distance %f = haversine_km(%f, %f, %f, %f)\n", plus_distance, data.latitude, data.longitude, _last_latitude, _last_longitude);
    }
    _last_latitude = data.latitude;
    _last_longitude = data.longitude;


    NS = data.latitude < 0 ? 'S' : 'N';
    WE = data.longitude < 0 ? 'W' : 'E';

    double latitude = dd_to_dm(data.latitude < 0 ? data.latitude * -1 : data.latitude);
    double longitude = dd_to_dm(data.longitude < 0 ? data.longitude * -1 : data.longitude);
    latitude_dm = static_cast<uint32_t>(latitude);
    latitude_s = static_cast<uint32_t>((latitude - latitude_dm) * 1e4);
    longitude_dm = static_cast<uint32_t>(longitude);
    longitude_s = static_cast<uint32_t>((longitude - longitude_dm) * 1e4);
  }


  bool gps_valid = gps_data.valid();
  bool dop_valid = gps_valid && gps_data.pdop * 100 < _settings.get_dop_max();

  sprintf(
      data.timestamp,
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      gps_data.year, gps_data.month, gps_data.day, gps_data.hour, gps_data.minute, gps_data.second);

  sprintf(
      data.log_string,
      "$%s,%04d,%s,%u,%u,%u,%c,%04d.%04d,%c,%04d.%04d,%c,%.1f,%c,%d,%d",
      DEVICE_HEADER, _settings.get_device_id(),
      data.timestamp,
      gm_sensor_data.cpm_comp, gm_sensor_data.cp5s, gm_sensor_data.total, gm_sensor_data.valid ? 'A' : 'V',
      latitude_dm, latitude_s, NS, longitude_dm, longitude_s, WE, data.altitude, gps_valid ? 'A' : 'V', gps_data.satsInView,
      static_cast<int>(100 * gps_data.pdop));  // DOP logged as integer, displayed as float 

  size_t len = strlen(data.log_string);
  data.log_string[len] = '\0';

  // generate checksum
  uint8_t chk = checksum(data.log_string + 1, len);

  // add checksum to end of line
  sprintf(data.log_string + len, "*%02X", chk);

  data.gps_valid = gps_data.valid();
  data.gm_valid = gm_sensor_data.valid;
  data.dop_valid = dop_valid;

  data.ibatt = pwrmon_data.ibatt;
  data.vbatt = pwrmon_data.vbatt;
  data.iboost = pwrmon_data.iboost;
  data.vboost = pwrmon_data.vboost;
  data.ibus = pwrmon_data.ibus;
  data.vbus = pwrmon_data.vbus;
  data.percentage = battery_data.percentage;
  data.isCharging = battery_data.isCharging;
  DEBUG_PRINTF("Log Aggregator: data.vbus = %f\n", data.vbus);

  return e_worker_data_read;
}
