#include "navsat_collector.h"


NavsatCollector::NavsatCollector(TeenyUbloxConnect& gnss) : Worker<NavsatData>({
                                                                .available=false,
                                                                .navsat_info=ubloxNAVSATInfo_t(),
                                                            }), _gnss(gnss) {
}

/**
 * @return true if auto NAVSAT has been set, only if GPS module is already initialized.
*/
bool NavsatCollector::activate(bool retry) {
  if (!retry && _gnss.pollUART1Port()) {
    _gnss.setAutoNAVSATRate(10);
    return true;
  }
  return false;
}

void NavsatCollector::deactivate() {
  _gnss.setAutoNAVSATRate(0);
}

int8_t NavsatCollector::produce_data() {
  // Collect NAVSAT Packet

  if (_gnss.getNAVSAT()) {
    _gnss.getNAVSATInfo(data.navsat_info);
    data.available = data.navsat_info.validPacket;

    return e_worker_data_read;
  }
  return e_worker_idle;
}
