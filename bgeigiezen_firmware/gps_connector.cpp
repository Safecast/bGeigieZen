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
    uint8_t c = 0;
    c = ss.read();
    Serial.printf("%c", c);
    gps.encode(c);
  }
  // Serial.printf("GPS chars processed: %d\n", gps.charsProcessed());

  if (gps.location.isValid()) {
    Serial.println("gps.location.isValid returned true.");
    data.available = true;
    data.updated = gps.location.isUpdated();// but not necessarily changed.
    data.age = gps.location.age();
    data.longitude = gps.location.lng();
    data.latitude = gps.location.lat();
    data.altitude = gps.altitude.meters();
    data.satellites = gps.satellites.value();
    return e_worker_data_read;
  } else {
    // Serial.println("gps.location.isValid returned false.");
    data.available = false;
    return e_worker_idle;
  }
}
