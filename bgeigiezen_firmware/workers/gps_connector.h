/** @brief Manage position fix and date-time from GNSS receiver. 
 * 
 * Based on u-blox 8 receiver integrated Beitian BN-880 module, using the UBX communications
 * protocol instead of the usual NMEA-183. The problem with NMEA is that it was really 
 * designed for boats, where close enough to see with binoculars was good enough (well, not
 * really but it makes for a good story). The real issue is that it's hard to get consistent
 * fix and time data by parsing NMEA sentences.
 * 
*/

#ifndef BGEIGIEZEN_GPS_SENSOR_H_
#define BGEIGIEZEN_GPS_SENSOR_H_

#include <Arduino.h>
#include <RBD_Timer.h>
#include <Worker.hpp>
#include <user_config.h>

// Sparkfun Electronics library v2 (original is deprecated, V3 is for devices newer than M8)
// Apply these two definitions here instead of altering the files in libdeps
// Uncomment the next line (or add SFE_UBLOX_REDUCED_PROG_MEM as a compiler directive) to reduce the amount of program memory used by the library
#define SFE_UBLOX_REDUCED_PROG_MEM // Uncommenting this line will delete the minor debug messages to save memory
// Uncomment the next line (or add SFE_UBLOX_DISABLE_AUTO_NMEA as a compiler directive) to reduce the amount of program memory used by the library
#define SFE_UBLOX_DISABLE_AUTO_NMEA // Uncommenting this line will disable auto-NMEA support to save memory
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_structs.h>  // structs internal to SparkFun library

struct GnssData {
  // When true, the item related to each Boolean is valid and updated in the
  // most recent poll of the gps_connector worker by the controller. If the 
  // gps_connector doesn't have a new fix because it only becomes available
  // every 1 second, then the corresponding Boolean becomes false.
  // If stale, render in gray/white text. 
  bool location_valid;
  bool satellites_valid;
  bool date_valid;
  bool time_valid;

  bool valid() const {
    return location_valid && satellites_valid && date_valid && time_valid;
  }

  // Position
  double latitude;  // Longitude: deg
  double longitude; // Longitude: deg
  double altitudeMSL;  // above mean sea level in meters

  // Confidence
  uint8_t satsInView;  // satellites used to calculate fix
  double hdop;


  // Date & Time
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

};

/**
 * GPS device worker, produces the current GPS location.
 */
class GpsConnector : public Worker<GnssData> {
 public:

  explicit GpsConnector(uint8_t gps_serial_num, SFE_UBLOX_GNSS& gnss);

  virtual ~GpsConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  /* u-blox UBX protocol query PVT gets position, velocity & time in one call.
   * The call returns false if no new fix has been received. In other words,
   * each call only succeeds once until the next fix update.
   * Use pdop and satsInView to determine acceptability.
  */
  SFE_UBLOX_GNSS& gnss;
  HardwareSerial ss;
  uint32_t tried_38400_at;
  uint32_t tried_9600_at;
  uint32_t _init_at;

  // UBX-NAV-SAT callback
  void satellites_callback(UBX_NAV_SAT_data_t *ubxDataStruct);

  // Age each item. If the corresponding timer times out, it's stale.
  RBD::Timer location_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer date_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_getpvt{GPS_FIX_AGE_LIMIT};  // if no response from getPVT()

};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
