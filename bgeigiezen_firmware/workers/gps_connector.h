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

  bool isValid() const {
    return (pdop >= GPS_PDOP_THRESHOLD) && (satsInView >= GPS_SATS_THRESHOLD);
  }

  // Confidence 
  uint8_t satsInView;
  int16_t pdop;  // Position dilution of precision (TBD is this a combination of HDOP and AltDOP?)

  // Position
  int32_t latitude;
  int32_t longitude;
  int32_t altitude;  // above ellipsoid (which one is better???)
  int32_t altitudeMSL;  // above mean sea level

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

  explicit GpsConnector();

  virtual ~GpsConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  HardwareSerial ss{GPS_SERIAL_NUM};
  SFE_UBLOX_GNSS gnss;
};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
