#include "navsat_collector.h"


NavsatCollector::NavsatCollector(TeenyUbloxConnect& gnss, const GnssData& gps_data) : Worker<NavsatData>({
                                                                .available=false,
                                                                .navsat_info=ubloxNAVSATInfo_t(),
                                                            }), _gnss(gnss), _gps_data(gps_data), _last_gsv_cycle(0) {
}

/**
 * @return true if auto NAVSAT has been set, only if GPS module is already initialized.
*/
bool NavsatCollector::activate(bool retry) {
  if (_gps_data.nmea_mode) {
    return true; // NMEA mode: always activate, data comes from GPS connector NMEA parsing
  }
  if (_gnss.pollUART1Port()) {
    return _gnss.setAutoNAVSATRate(10);
  }
  return false;
}

void NavsatCollector::deactivate() {
  if (_gps_data.nmea_mode) {
    return;
  }
  _gnss.setAutoNAVSAT(false);
}

int8_t NavsatCollector::produce_data() {
  if (_gps_data.nmea_mode) {
    // Build navsat_info from NMEA $GPGSV/$GPGSA data collected by GpsConnector.
    // Only fire once per complete GPGSV cycle to avoid flickering the satellite view.
    if (_gps_data.nmea_gsv_cycle == _last_gsv_cycle || _gps_data.nmea_sat_count == 0) return e_worker_idle;
    _last_gsv_cycle = _gps_data.nmea_gsv_cycle;
    auto& info = data.navsat_info;
    info.validPacket = true;
    info.numSvs = min(_gps_data.nmea_sat_count, (uint8_t)UBX_MAXNAVSATSATELLITES);
    info.numSvsReceived = info.numSvs;
    info.numSvsEphValid = info.numSvs;
    info.numSvsHealthy = 0;
    info.numSvsUsed = 0;
    for (uint8_t i = 0; i < info.numSvs; i++) {
      const auto& s = _gps_data.nmea_sats[i];
      info.svSortList[i].svId       = s.svId;
      info.svSortList[i].gnssIdType = s.gnssIdType;
      info.svSortList[i].elev       = s.elev;
      info.svSortList[i].azim       = s.azim;
      info.svSortList[i].cno        = s.cno;
      info.svSortList[i].svUsed     = s.svUsed;
      info.svSortList[i].healthy    = (s.cno > 0);
      info.svSortList[i].ephValid   = true;
      if (s.svUsed)  info.numSvsUsed++;
      if (s.cno > 0) info.numSvsHealthy++;
    }
    data.available = true;
    return e_worker_data_read;
  }

  // UBX mode: collect NAV-SAT packet from module
  if (_gnss.getNAVSAT()) {
    _gnss.getNAVSATInfo(data.navsat_info);
    data.available = data.navsat_info.validPacket;
    return e_worker_data_read;
  }
  return e_worker_idle;
}
