#ifndef BGEIGIEZEN_GPS_SENSOR_HPP
#define BGEIGIEZEN_GPS_SENSOR_HPP

#include <Arduino.h>
#include <Worker.hpp>

struct GpsData {
  bool available;
  uint8_t satellites;
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

};

#endif //BGEIGIEZEN_GPS_SENSOR_HPP
