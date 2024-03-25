
#ifndef BGEIGIEZEN_NAVSAT_COLLECTOR_H_
#define BGEIGIEZEN_NAVSAT_COLLECTOR_H_

#include <Worker.hpp>
#include <TeenyUbloxConnect.h>


struct NavsatData {
  bool available;
  ubloxPacket_t navsat_packet;
  ubloxNAVSATInfo_t navsat_info;

};

/**
 * GPS device worker, produces the current GPS location.
 */
class NavsatCollector : public Worker<NavsatData> {
 public:

  explicit NavsatCollector(TeenyUbloxConnect& _gnss);

  virtual ~NavsatCollector() = default;

  bool activate(bool retry) override;

  void deactivate() override;

  int8_t produce_data() override;

 private:
  TeenyUbloxConnect& _gnss;
};

#endif //BGEIGIEZEN_NAVSAT_COLLECTOR_H_
