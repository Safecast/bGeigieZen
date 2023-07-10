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

  if (gps.location.isValid() && gps.satellites.isValid() && gps.date.isValid() && gps.time.isValid()) {
    data.available = true;
    data.updated = gps.location.isUpdated(); // but not necessarily changed.
    data.age = gps.location.age();
    data.longitude = gps.location.lng();
    data.latitude = gps.location.lat();
    data.altitude = gps.altitude.meters();
    data.satellites = gps.satellites.value();

    data.year = gps.date.year();
    data.month = gps.date.month();
    data.day = gps.date.day();
    data.hour = gps.time.hour();
    data.minute = gps.time.minute();
    data.second = gps.time.second();
    return e_worker_data_read;
  }
  else {
    data.available = false;
    return e_worker_idle;
  }
}
