/** @brief GNSS handler using UBX protocol library 
 * 
 * @todo Split into a date & time producer and a position producer
 * The date and time are received sooner than position because fewer
 * satellites are needed for time information. This means that we can set
 * the clock and open log files before the full 3D position fix is ready.
 * Eventually, gps_connector is separate and there are two workers, datetime
 * and position, each pulling information from gps_connector.
 * With Core2 (and Core with external RTC), date-time can be obtained from
 * the RTC before GNSS receives time. In that case, the date-time worker can
 * choose the more reliable of the two: GNSS preferred, RTC otherwise.
 * We also have the issue of initializing RTC on a brand new device, that 
 * can be done from GNSS.
*/

#include "gps_connector.h"

GpsConnector::GpsConnector() : Worker<GnssData>() {
}
/**
 * @return true if initialized GNSS library, false if no connection with module.
*/
bool GpsConnector::activate(bool retry) {
  // From Sparkfun examples/Example12_UseUart
  // Assume that the U-Blox GNSS is running at 9600 baud (the default) or at 38400 baud.
  // Loop until we're in sync and then ensure it's at 38400 baud.
  int loopcount = 0; // prevent an infinite loop
  do {
    Serial.println("GNSS: trying 38400 baud");
    ss.begin(38400);
    if (gnss.begin(ss) == true)
      break;
    delay(100);
    Serial.println("GNSS: trying 9600 baud");
    ss.begin(9600);
    if (gnss.begin(ss) == true) {
        Serial.println("GNSS: connected at 9600 baud, switching to 38400");
        gnss.setSerialRate(38400);
        delay(100);
    }
    else {
      delay(500);
    }
    ++loopcount;
    if(loopcount > 5) {
      Serial.println("Cannot connect to GNSS module.");
      return false;
    }
  } while(1);

  // Set Auto on NAV-PVT and NAV-DOP queries for non-blocking access
  // getPVT() and getDOP() will return true if a new navigation solution is available
  gnss.setAutoPVT(true); // Tell the GNSS to "send" each solution
  gnss.setAutoDOP(true); // Enable/disable automatic DOP reports at the navigation frequency

  // Mark the fix items invalid to start
  data.location_valid = false;
  data.altitude_valid = false;
  data.date_valid = false;
  data.time_valid = false;

  return true;
}

int8_t GpsConnector::produce_data() {

  auto status = e_worker_idle;

  // getPVT and getDOP will return true if there actually is a fresh navigation solution
  // available. "LLH" is longitude, latitude, height.
  // getPVT() returns UTC date and time (do not use GNSS time, see note in u-blox spec)
  if (gnss.getPVT() && gnss.getDOP())
  {
    data.time_getpvt.restart();

    // Satellitesis a special case, we want to see it even if no fix.
    data.satsInView = gnss.getSIV();  // Satellites In View
    data.satellites_valid = true;

    if(gnss.getDateValid())
    {
      data.year = gnss.getYear();
      data.month = gnss.getMonth();
      data.day = gnss.getDay();
      data.date_valid = true;
      data.date_timer.restart();
    } else {
      if(data.date_timer.isExpired()) {
        data.date_valid = false;
      }
    }

    if(gnss.getTimeValid())
    {
      data.hour = gnss.getHour();
      data.minute = gnss.getMinute();
      data.second = gnss.getSecond();
      data.time_valid = true;
      data.time_timer.restart();
    } else {
      if(data.time_timer.isExpired()) {
        data.time_valid = false;
      }
    }
    if(gnss.getGnssFixOk())
    {
      // data.satsInView = gnss.getSIV();  // Satellites In View
      data.hdop = gnss.getHorizontalDOP();  // Position Dilution of Precision
      data.latitude = gnss.getLatitude();
      data.longitude = gnss.getLongitude();
      data.altitudeMSL = gnss.getAltitudeMSL(); // Above MSL (not ellipsoid)
      data.location_valid = true;
      data.location_timer.restart();
    } else {
      if(data.location_timer.isExpired()) {
        data.location_valid = false;
      }
    }
    status =  e_worker_data_read;
  }
  else {
    status = e_worker_idle;
    if(data.time_getpvt.isExpired()) {
      data.satellites_valid = false;
      data.location_valid = false;
      data.altitude_valid = false;
      data.date_valid = false;
      data.time_valid = false;
    }
  }

  return status;
}
