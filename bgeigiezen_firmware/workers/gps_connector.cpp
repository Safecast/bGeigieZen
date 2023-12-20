/** @brief GNSS handler using UBX protocol library 
 * 
 * When activated, set auto receive for the UBX protocol commands used:
 * UBX-NAV-PVT: position, velocity and time
 * ***REMOVED*** UBX-NAV-DOP: dilution of precision, for horizontal DOP
 * ***REMOVED*** UBX-NAV-SAT: enumeration of satellites in view, for number of sats.
 * Based on examples by Paul Clark, SparkFun Electronics
 * https://github.com/sparkfun/SparkFun_u-blox_GNSS_Arduino_Library
 * 
 * ***REMOVED*** The two removed packets were not providing any additional
 * useful information. HDOP was always zero. NSATS was the total number of
 * satellites, regardless of their status in computing a fix.
*/

#include "gps_connector.h"
#include "debugger.h"


GpsConnector::GpsConnector(uint8_t gps_serial_num, SFE_UBLOX_GNSS& gnss) : Worker<GnssData>(900), gnss(gnss), ss(gps_serial_num), tried_38400_at(0), tried_9600_at(0), _init_at(0) {
}
/**
 * @return true if initialized GNSS library, false if no connection with module.
*/
bool GpsConnector::activate(bool retry) {

  // From Sparkfun examples/Example12_UseUart
  // Assume that the U-Blox GNSS is running at 9600 baud (the default) or at 38400 baud.
  if (!retry) {
    ss.begin(38400);
    _init_at = millis();
  }
  if (tried_38400_at == 0 && (millis() - _init_at > 500)) { // Wait for device to completely startup
    tried_38400_at = millis();
    ss.updateBaudRate(38400);
    if (gnss.begin(ss, 500)) {
      DEBUG_PRINTLN("GNSS: connected at 38400 baud");
      gnss.setSerialRate(38400);
    }
    else {
      return false;
    }
  }
  else if (tried_9600_at == 0 && tried_38400_at > 0 && (millis() - tried_38400_at > 500)) {
    tried_9600_at = millis();
    ss.updateBaudRate(9600);
    if (gnss.begin(ss, 500)) {
      DEBUG_PRINTLN("GNSS: connected at 9600 baud, switching to 38400");
      gnss.setSerialRate(38400);
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }

  // Confirm that we actually have a connection
  DEBUG_PRINTF("GNSS: u-blox protocol version %02d.%02d\n",
                gnss.getProtocolVersionHigh(),
                gnss.getProtocolVersionLow());

  // Send UBX, disable NMEA-0183 messages that we are ignoring anyway.
  gnss.setPortOutput(COM_PORT_UART1, COM_TYPE_UBX);

  // Set Auto on NAV-PVT for non-blocking access
  // getPVT() will return true if a new navigation solution is available
  gnss.setAutoPVT(true); // Tell the GNSS to send the solution as it is computed (1 second)

 // Mark the fix items invalid to start
  data.location_valid = false;
  data.date_valid = false;
  data.time_valid = false;

  return true;
}

int8_t GpsConnector::produce_data() {
  auto ret_status = e_worker_idle;

  // getPVT returns true if there is a fresh navigation solution available.
  // "LLH" is longitude, latitude, height.
  // getPVT() returns UTC date and time.
  // Do not use GNSS time, see u-blox spec section 9.
  if (gnss.getPVT()) {
    // DEBUG_PRINTF("[%d] gnss.getPVT() && gnss.getDOP() && gnss.getNAVSAT() is true.\n", millis());
    location_timer.restart();

    if (gnss.getGnssFixOk()) {
      // DEBUG_PRINTF("[%d] gnss.getGnssFixOk() is true.\n", millis());
      data.pdop = gnss.getPDOP(); // Position Dilution of Precision
      data.latitude = gnss.getLatitude() * 1e-7;
      data.longitude = gnss.getLongitude() * 1e-7;
      data.altitudeMSL = gnss.getAltitudeMSL() * 1e-3; // Above MSL (not ellipsoid)
      data.location_valid = true;
      data.satsInView = gnss.getSIV(); // Satellites In View
      location_timer.restart();
      ret_status = e_worker_data_read;
      // DEBUG: Compare PDOP from NAV-PVT and HDOP from NAV-DOP
      // DEBUG_PRINTF("[%d] GnssFixOk (type = %d).\n"
      //               "  SATS: %d; PDOP: %d; HDOP: %d\n",
      //               millis(), gnss.getFixType(),
      //               data.satsInView, gnss.getPDOP(), data.hdop);
    }
    else {
      // No valid fix, report zero satellites.
      data.satsInView = 0; // Satellites used in fix
      ret_status = e_worker_data_read;
      if (location_timer.isExpired()) {
        data.location_valid = false;
      }
    }

    if (gnss.getDateValid()) {
//      DEBUG_PRINTF("[%lu] gnss.getDateValid() is true.\n", millis());
      data.year = gnss.getYear();
      data.month = gnss.getMonth();
      data.day = gnss.getDay();
      data.date_valid = true;
      date_timer.restart();
      ret_status = e_worker_data_read;
    }

    if (gnss.getTimeValid()) {
//      DEBUG_PRINTF("[%lu] gnss.getTimeValid() is true.\n", millis());
      data.hour = gnss.getHour();
      data.minute = gnss.getMinute();
      data.second = gnss.getSecond();
      data.time_valid = true;
      time_timer.restart();
      ret_status = e_worker_data_read;
    }
  }

  // Check expiry
  if (location_timer.isExpired()) {
    data.location_valid = false;
  }
  if (time_timer.isExpired()) {
    data.time_valid = false;
  }
  if (date_timer.isExpired()) {
    data.date_valid = false;
  }

  return ret_status;
}
