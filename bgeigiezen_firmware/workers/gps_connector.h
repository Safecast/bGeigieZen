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

#include <TeenyUbloxConnect.h>


struct GnssData {
  // When true, the item related to each Boolean is valid and updated in the
  // most recent poll of the gps_connector worker by the controller. If the 
  // gps_connector doesn't have a new fix because it only becomes available
  // every 1 second, then the corresponding Boolean becomes false.
  // If stale, render in gray/white text. 
  bool location_valid;
  bool date_valid;
  bool time_valid;

  bool valid() const {
    return location_valid && date_valid && time_valid;
  }

  enum Heading {
    UNKNOWN,
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
  };

  // Position
  double latitude;  // Longitude: deg
  double longitude; // Longitude: deg
  double altitudeMSL;  // above mean sea level in meters
  double heading_degree; // in degrees
  Heading heading;

  // Confidence
  uint8_t satsInView;  // satellites used to calculate fix
  double pdop;  // position dilution of precision (not horizontal)

  // Date & Time
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  // Debug info
  uint8_t protocolVersionHigh;
  uint8_t protocolVersionLow;

};

/**
 * GPS device worker, produces the current GPS location.
 */
class GpsConnector : public Worker<GnssData> {
 public:

  explicit GpsConnector(TeenyUbloxConnect& _gnss, HardwareSerial& serial);

  virtual ~GpsConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 protected:
  void deactivate() override;

 private:
  /* u-blox UBX protocol query PVT gets position, velocity & time in one call.
   * The call returns false if no new fix has been received. In other words,
   * each call only succeeds once until the next fix update.
   * Use pdop and satsInView to determine acceptability.
  */
  TeenyUbloxConnect& _gnss;
  HardwareSerial& _serial_conn;
  uint32_t _tried_38400_at;
  uint32_t _tried_9600_at;
  uint32_t _init_at;

  // Age each item. If the corresponding timer times out, it's stale.
  RBD::Timer location_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer date_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_timer{GPS_FIX_AGE_LIMIT};
  RBD::Timer time_getpvt{GPS_FIX_AGE_LIMIT};  // if no response from getPVT()
};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
