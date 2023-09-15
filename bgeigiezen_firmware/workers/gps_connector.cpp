/** @brief GNSS handler using UBX protocol library 
 * 
 * @todo Split into a date & time producer and a position producer
 * The date and time are received sooner than position because only one
 * satellite is needed for time information. This means that we can set
 * the clock and open log files before the full 3D position fix is ready.
 * Eventually, gps_connector is separate and there are two workers, datetime
 * and position, each pulling information from gps_connector.
 * With Core2 (and Core with external RTC), date-time can be obtained from
 * the RTC before GNSS receives time. In that case, the date-time worked can
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
        ++loopcount;
        delay(100);
    } else {
        if(loopcount > 5)
          return false;
        else
          delay(500);
    }
  } while(1);
  return true;
}

int8_t GpsConnector::produce_data() {

  auto status = e_worker_idle;

  // getPVT will return true if there actually is a fresh navigation solution available.
  // Important note: the PVT message is 100 bytes long. We need to call getPVT often enough
  // to prevent serial buffer overflows on boards like the original RedBoard / UNO.
  // At 38400 Baud, the 100 PVT bytes will arrive in 26ms.
  // On the RedBoard, we need to call getPVT every 5ms to keep up.
  if (gnss.getPVT() && gnss.getDateValid() )   // && (gnss.getInvalidLlh() == false)
  {
    data.satsInView = gnss.getSIV();  // Satellites In View
    data.pdop = gnss.getPDOP();  // Position Dilution of Precision
    if((data.satsInView >= GPS_SATS_THRESHOLD) && (data.pdop >= GPS_PDOP_THRESHOLD)) {
      data.latitude = gnss.getLatitude();
      data.longitude = gnss.getLongitude();
      data.altitude = gnss.getAltitude();
      data.altitudeMSL = gnss.getAltitudeMSL();
      data.satsInView = gnss.getSIV();  // Satellites In View
      data.pdop = gnss.getPDOP();  // Position Dilution of Precision

      data.year = gnss.getYear();
      data.month = gnss.getMonth();
      data.day = gnss.getDay();

      data.hour = gnss.getHour();
      data.minute = gnss.getMinute();
      data.second = gnss.getSecond();
      status =  e_worker_data_read;
    }
    else {
      status = e_worker_idle;
    }
  }

  return status;
}
