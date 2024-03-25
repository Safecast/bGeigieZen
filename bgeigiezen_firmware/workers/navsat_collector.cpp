#include "navsat_collector.h"


NavsatCollector::NavsatCollector(TeenyUbloxConnect& gnss) : Worker<NavsatData>({
                                                                .available=false,
                                                                .navsat_packet=ubloxPacket_t(),
                                                                .navsat_info=ubloxNAVSATInfo_t(),
                                                            }), _gnss(gnss) {
}

/**
 * @return true if auto NAVSAT has been set, only if GPS module is already initialized.
*/
bool NavsatCollector::activate(bool retry) {
  if (!retry && _gnss.getProtocolVersion()) {
    _gnss.setAutoNAVSAT(true);
    return true;
  }
  return false;
}

void NavsatCollector::deactivate() {
  _gnss.setAutoNAVSAT(false);
}

int8_t NavsatCollector::produce_data() {
  // Collect NAVSAT Packet

  if (_gnss.getNAVSAT()) {
    _gnss.getNAVSATPacket(data.navsat_packet);
    _gnss.getNAVSATInfo(data.navsat_info);
    data.available = data.navsat_packet.validPacket;

    return e_worker_data_read;
  }
  return e_worker_idle;
}
