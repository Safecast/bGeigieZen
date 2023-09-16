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
#include <Worker.hpp>
#include <user_config.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

struct GnssData {
  /* u-blox UBX protocol query PVT gets position, velocity & time in one call.
   * The call returns false if no new fix has been received. In other words, 
   * each call only succeeds once until the next fix update.
   * Use pdop and satsInView to determine acceptability. 
  */

  // Confidence 
  uint8_t satsInView;
  // Position dilution of precision * 0.01 (TBD, is this a combination of HDOP and AltDOP?) 
  int16_t pdop;
  // Temporary until the various levels of confidence can be established
  bool location_valid;
  bool altitude_valid;
  bool satellites_valid;
  bool date_valid;
  bool time_valid;

  // Position
  int32_t latitude;  // Longitude: deg * 1e-7
  int32_t longitude; // Longitude: deg * 1e-7
  int32_t altitude;  // above ellipsoid mm (which one is better???)
  int32_t altitudeMSL;  // above mean sea level mm

  // Date & Time
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  bool isValid() const {
    return (pdop < GPS_PDOP_THRESHOLD) && (satsInView >= GPS_SATS_THRESHOLD);
  }

};

/**
 * GPS device worker, produces the current GPS location.
 */
class GpsConnector : public Worker<GnssData> {
 public:

  explicit GpsConnector();

  virtual ~GpsConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  HardwareSerial ss{GPS_SERIAL_NUM};
  SFE_UBLOX_GNSS gnss;
};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
