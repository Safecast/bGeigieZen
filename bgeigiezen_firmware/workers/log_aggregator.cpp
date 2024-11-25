#include "utils/functions.h"
#include "log_aggregator.h"
#include "battery_indicator.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "identifiers.h"

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
      //      M5_LOGD("Distance %f = haversine_km(%f, %f, %f, %f)", plus_distance, data.latitude, data.longitude, _last_latitude, _last_longitude);
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

  snprintf(
      data.timestamp, sizeof(data.timestamp),
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      gps_data.year, gps_data.month, gps_data.day, gps_data.hour, gps_data.minute, gps_data.second);

  snprintf(
      data.log_working_string, sizeof(data.log_string),
      "$%s,%04d,%s,%u,%u,%u,%c,%04d.%04d,%c,%04d.%04d,%c,%.1f,%c,%d,%d",
      DEVICE_HEADER, _settings.get_device_id(),
      data.timestamp,
      gm_sensor_data.cpm_comp, gm_sensor_data.cp5s, gm_sensor_data.total, gm_sensor_data.valid ? 'A' : 'V',
      latitude_dm, latitude_s, NS, longitude_dm, longitude_s, WE, data.altitude, gps_valid ? 'A' : 'V', gps_data.satsInView,
      static_cast<int>(100 * gps_data.pdop));  // DOP logged as integer, displayed as float 

  size_t len = strlen(data.log_working_string);
  // This is redundant now that snprintf() is used.
  // data.log_string[len] = '\0';

  // generate checksum and format the end of the line
  uint8_t chk = checksum(data.log_string + 1, len);
  snprintf(data.log_chksum_string, sizeof(data.log_chksum_string), "*%02X", chk);

  /*** 
   * Second line of log for extra troubleshooting info
   * Prefix $BNXNAV BgeigieNanoeXtraNAV info
   * ***/
  snprintf(
    data.log_secondary_string, sizeof(data.log_secondary_string),
    "$BNXNAV,%u,%u,%d,%d,%d,%d,%d,%u,%u,%c",
    gps_data.hAcc,  // mm Horizontal accuracy estimate for Long/Lat
    gps_data.vAcc,  // mm Vertical accuracy estimate for Long/Lat
    gps_data.velN,  // mm/s NED north velocity
    gps_data.velE,  // mm/s NED east velocity
    gps_data.velD,  // mm/s NED down velocity
    gps_data.gSpeed,  // Ground Speed (2-D)
    gps_data.headMot,  // Heading of motion (2-D)
    gps_data.sAcc,
    gps_data.headAcc,
    gps_data.invalidLlh?'1':'0'  // NAVPVT[78] flags3 bit 0
    );

  // Put it all together
  snprintf(data.log_string, sizeof(data.log_string),
        "%s%s\n%s",
        data.log_working_string,
        data.log_chksum_string,
        data.log_secondary_string
        );

  data.gps_valid = gps_data.valid();
  data.gm_valid = gm_sensor_data.valid;
  data.dop_valid = dop_valid;

  return e_worker_data_read;
}
