#include "gps_connector.h"

GpsConnector::GpsConnector() : Worker<GpsData>() {
}

bool GpsConnector::activate(bool retry) {
  // open gps module connection
  ss.begin(GPS_BAUD_RATE);
  return true;
}

int8_t GpsConnector::produce_data() {
  // Poll the serial port and feed the encoder
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  auto status = e_worker_idle;

  if (gps.location.isUpdated()) {
    data.location_valid = gps.location.isValid();
    data.longitude = gps.location.lng();
    data.latitude = gps.location.lat();
    status = e_worker_data_read;
  }

  if (gps.altitude.isUpdated()) {
    data.altitude_valid = gps.altitude.isValid();
    data.altitude = gps.altitude.meters();
    status = e_worker_data_read;
  }

  if (gps.satellites.isUpdated()) {
    data.satellites_valid = gps.satellites.isValid();
    data.satellites_value = gps.satellites.value();
    return e_worker_data_read;
  }

  if (gps.date.isUpdated()) {
    data.date_valid = gps.date.isValid();
    data.year = gps.date.year();
    data.month = gps.date.month();
    data.day = gps.date.day();
    return e_worker_data_read;
  }

  if (gps.time.isUpdated()) {
    data.time_valid = gps.time.isValid();
    data.hour = gps.time.hour();
    data.minute = gps.time.minute();
    data.second = gps.time.second();
    return e_worker_data_read;
  }

  // Always update age
  data.location_age = gps.location.age();
  data.location_age = gps.location.age();
  data.altitude_age = gps.altitude.age();
  data.satellites_age = gps.satellites.age();
  data.date_age = gps.date.age();

  return status;
}
