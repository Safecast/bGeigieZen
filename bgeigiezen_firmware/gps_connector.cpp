#include "gps_connector.h"


GpsConnector::GpsConnector() : Worker<GpsData>() {
}

bool GpsConnector::activate(bool retry) {
  // open gps module connection
  return true;
}

int8_t GpsConnector::produce_data() {
  data.available = true;
  data.longitude = 12.123;
  data.latitude = 12.123;
  data.altitude = 0.123;
  return e_worker_idle;
}
