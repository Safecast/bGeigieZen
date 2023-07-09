#ifndef BGEIGIEZEN_GPS_SENSOR_HPP
#define BGEIGIEZEN_GPS_SENSOR_HPP

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
  bool available;// based on gps.isValid()
  bool updated;  // based on gps.isUpdated(), fresh but not necessarily changed.
  uint32_t age;  // milliseconds, based on gps.age()
  uint32_t satellites;
  double longitude;
  double latitude;
  double altitude;
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

#endif //BGEIGIEZEN_GPS_SENSOR_HPP
