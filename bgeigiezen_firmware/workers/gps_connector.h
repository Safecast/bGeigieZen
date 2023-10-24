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

struct GnssData {
  /* u-blox UBX protocol query PVT gets position, velocity & time in one call.
   * The call returns false if no new fix has been received. In other words, 
   * each call only succeeds once until the next fix update.
   * Use pdop and satsInView to determine acceptability. 
  */

  // Confidence 
  uint8_t satsInView;  // satellites used to calculate fix
  // Horizontal dilution of precision * 0.01 from NAV-DOP
  int16_t hdop;

  // When true, the item related to each Boolean is valid and updated in the
  // most recent poll of the gps_connector worker by the controller. If the 
  // gps_connector doesn't have a new fix because it only becomes available
  // every 1 second, then the corresponding Boolean becomes false.
  // If stale, render in gray/white text. 
  bool location_valid;
  bool altitude_valid;
  bool satellites_valid;
  bool date_valid;
  bool time_valid;

  /** @todo REMOVE THIS DEBUG TEMPORARY*/
  RBD::Timer hardreset_timer{15000};  // force a GNSS module hard reset to see the effects.

  // Age each item. If the corresponding timer times out, it's stale. 
  RBD::Timer location_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer altitude_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer satellites_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer date_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_getpvt{GPS_FIX_AGE_LIMIT};  // if no response from getPVT()

  // Position
  int32_t latitude;  // Longitude: deg * 1e-7
  int32_t longitude; // Longitude: deg * 1e-7
  int32_t altitudeMSL;  // above mean sea level mm

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
  HardwareSerial ss;
  SFE_UBLOX_GNSS& gnss;
  uint32_t tried_38400_at;
  uint32_t tried_9600_at;
};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
