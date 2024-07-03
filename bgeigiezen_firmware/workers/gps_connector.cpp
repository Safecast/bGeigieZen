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

#define GPS_INVALID_YEAR 2000
#define GPS_INVALID_MONTH 1
#define GPS_INVALID_DAY 1
#define GPS_INVALID_HOUR 0
#define GPS_INVALID_MINUTE 0
#define GPS_INVALID_SECOND 0

#define NNE 22.5
#define ENE (NNE + 45)
#define ESE (ENE + 45)
#define SSE (ESE + 45)
#define SSW (SSE + 45)
#define WSW (SSW + 45)
#define WNW (WSW + 45)
#define NNW (WNW + 45)

GpsConnector::GpsConnector(TeenyUbloxConnect& gnss) : Worker<GnssData>({
                                                       .location_valid=false,
                                                       .date_valid=false,
                                                       .time_valid=false,
                                                       .latitude=0,
                                                       .longitude=0,
                                                       .altitudeMSL=0,
                                                       .heading_degree=0,
                                                       .heading=GnssData::UNKNOWN,
                                                       .satsInView=0,
                                                       .pdop=0,
                                                       .year=0,
                                                       .month=0,
                                                       .day=0,
                                                       .hour=0,
                                                       .minute=0,
                                                       .second=0,
                                                       .protocolVersionHigh=0,
                                                       .protocolVersionLow=0
                                                   }), _gnss(gnss), ss(GPS_SERIAL_NUM), tried_38400_at(0), tried_9600_at(0), _init_at(0) {
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
  if (tried_38400_at == 0 && (millis() - _init_at > 1000)) { // Wait for device to completely startup
    tried_38400_at = millis();
    ss.updateBaudRate(38400);
    DEBUG_PRINTLN("GNSS: Try at 38400 baud");
    if (_gnss.begin(ss, 500)) {
      DEBUG_PRINTLN("GNSS: connected at 38400 baud"); // no need to set module to 38.4
    }
    else {
      return false;
    }
  }
  else if (tried_9600_at == 0 && tried_38400_at > 0 && (millis() - tried_38400_at > 1200)) {
    tried_9600_at = millis();
    ss.updateBaudRate(9600);
    DEBUG_PRINTLN("GNSS: Try at 9600 baud");
    if (_gnss.begin(ss, 500)) {
      DEBUG_PRINTLN("GNSS: connected at 9600 baud, switching to 38400");
      _gnss.setSerialRate(38400);
      delay(100); // recovery time for gnss module baud rate change
      ss.updateBaudRate(38400);
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }

  // Confirm that we actually have a connection
  data.protocolVersionHigh = _gnss.getProtocolVersionHigh();
  data.protocolVersionLow = _gnss.getProtocolVersionLow();
  DEBUG_PRINTF("GNSS: u-blox protocol version %02d.%02d\n",
              data.protocolVersionHigh, data.protocolVersionLow);

  // Send UBX, disable NMEA-0183 messages that we are ignoring anyway.
  _gnss.setPortOutput(COM_PORT_UART1, COM_TYPE_UBX);

  // Set Auto on NAV-PVT for non-blocking access
  // getPVT() will return true if a new navigation solution is available
  _gnss.setAutoPVT(true); // Tell the GNSS to send the solution as it is computed (1 second)
  _gnss.setAutoNAVSAT(false); // Disable navsat by default (navsat worker handles this)

  return true;
}

void GpsConnector::deactivate() {
  tried_9600_at = 0;
  tried_38400_at = 0;
  ss.end(true);
}

int8_t GpsConnector::produce_data() {
  auto ret_status = e_worker_idle;
  int i = 0;


  // getPVT returns true if there is a fresh navigation solution available.
  // "LLH" is longitude, latitude, height.
  // getPVT() returns UTC date and time.
  // Do not use GNSS time, see u-blox spec section 9.
  if (_gnss.getPVT()) {
    // DEBUG_PRINTF("[%d] _gnss.getPVT() is true.\n", millis());

    data.satsInView = _gnss.getSIV(); // Satellites In View

    if (_gnss.getFixType() == 2 || _gnss.getFixType() == 3) {
      // DEBUG_PRINTF("[%d] fix type is 2D or 3D.\n", millis());
      data.pdop = _gnss.getPDOP() * 1e-2; // Position Dilution of Precision
      data.latitude = _gnss.getLatitude() * 1e-7;
      data.longitude = _gnss.getLongitude() * 1e-7;
      data.heading_degree = _gnss.getHeading() * 1e-5;
      if (data.heading_degree >= NNW || data.heading_degree < NNE) {
        data.heading = GnssData::NORTH;
      }
      if (data.heading_degree >= NNE || data.heading_degree < ENE) {
        data.heading = GnssData::NORTHEAST;
      }
      if (data.heading_degree >= ENE || data.heading_degree < ESE) {
        data.heading = GnssData::EAST;
      }
      if (data.heading_degree >= ESE || data.heading_degree < SSE) {
        data.heading = GnssData::SOUTHEAST;
      }
      if (data.heading_degree >= SSE || data.heading_degree < SSW) {
        data.heading = GnssData::SOUTH;
      }
      if (data.heading_degree >= SSW || data.heading_degree < WSW) {
        data.heading = GnssData::SOUTHWEST;
      }
      if (data.heading_degree >= WSW || data.heading_degree < WNW) {
        data.heading = GnssData::WEST;
      }
      if (data.heading_degree >= WNW || data.heading_degree < NNW) {
        data.heading = GnssData::NORTHWEST;
      }
      data.altitudeMSL = _gnss.getFixType() == 3 ? _gnss.getAltitudeMSL() * 1e-3 : 0; // Above MSL (not ellipsoid)
      data.location_valid = _gnss.getGnssFixOk();
      location_timer.restart();
      ret_status = e_worker_data_read;
      // DEBUG: Compare PDOP from NAV-PVT and HDOP from NAV-DOP
      // DEBUG_PRINTF("[%d] GnssFixOk (type = %d).\n"
      //               "  SATS: %d; PDOP: %d; HDOP: %d\n",
      //               millis(), _gnss.getFixType(),
      //               data.satsInView, _gnss.getPDOP(), data.hdop);
    }

    if (_gnss.getDateValid()) {
//      DEBUG_PRINTF("[%lu] _gnss.getDateValid() is true.\n", millis());
      data.year = _gnss.getYear();
      data.month = _gnss.getMonth();
      data.day = _gnss.getDay();
      data.date_valid = true;
      date_timer.restart();
      ret_status = e_worker_data_read;
    }

    if (_gnss.getTimeValid()) {
//      DEBUG_PRINTF("[%lu] _gnss.getTimeValid() is true.\n", millis());
      data.hour = _gnss.getHour();
      data.minute = _gnss.getMinute();
      data.second = _gnss.getSecond();
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
    data.hour = GPS_INVALID_HOUR;
    data.minute = GPS_INVALID_MINUTE;
    data.second = GPS_INVALID_SECOND;
  }
  if (date_timer.isExpired()) {
    data.date_valid = false;
    data.year = GPS_INVALID_YEAR;
    data.month = GPS_INVALID_MONTH;
    data.day = GPS_INVALID_DAY;
  }

  return ret_status;
}
