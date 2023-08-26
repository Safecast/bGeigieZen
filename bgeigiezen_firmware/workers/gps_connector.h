#ifndef BGEIGIEZEN_GPS_SENSOR_H_
#define BGEIGIEZEN_GPS_SENSOR_H_

#include <Arduino.h>
#include <Worker.hpp>
#include <user_config.h>
#include <TinyGPS++.h>

struct GpsData {
  /* Validity, Update status, and Age
  You can examine an object’s value at any time, but unless TinyGPS++ has
  recently been fed from the GPS, it should not be considered valid and 
  up-to-date. The isValid() method will tell you whether the object contains
  any valid data and is safe to query.
  Similarly, isUpdated() indicates whether the object’s value has been updated
  (not necessarily changed) since the last time you queried it.
  Lastly, if you want to know how stale an object’s data is, call its age()
  method, which returns the number of milliseconds since its last update.
  If this returns a value greater than 1500 or so, it may be a sign of a
  problem like a lost fix.
  Ref: http://arduiniana.org/libraries/tinygpsplus/
  */

  bool isValid() const {
    return location_valid && altitude_valid && satellites_valid && date_valid;
  }

  // location
  bool location_valid;
  bool location_age;
  double longitude;
  double latitude;

  // Altitude
  bool altitude_valid;
  bool altitude_age;
  double altitude;

  // Satellites
  bool satellites_valid;
  bool satellites_age;
  uint32_t satellites_value;

  // Date
  bool date_valid;
  bool date_age;
  uint16_t year;
  uint8_t month;
  uint8_t day;

  // Time
  bool time_valid;
  bool time_age;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

/**
 * GPS device worker, produces the current GPS location.
 * Uses TinyGPS?
 */
class GpsConnector : public Worker<GpsData> {
 public:

  explicit GpsConnector();

  virtual ~GpsConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  HardwareSerial ss{GPS_SERIAL_NUM};
  TinyGPSPlus gps;
};

#endif //BGEIGIEZEN_GPS_SENSOR_H_
