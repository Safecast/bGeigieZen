#ifndef BGEIGIEZEN_GEIGERSENSOR_HPP
#define BGEIGIEZEN_GEIGERSENSOR_HPP

#include <Worker.hpp>

struct GMData {
  uint32_t CPM;
  double ush;
};

/**
 * Geiger counter worker, produces CPM.
 */
class GeigerMullerSensor : public Worker<GMData> {
 public:

  explicit GeigerMullerSensor();

  virtual ~GeigerMullerSensor() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:

};

#endif //BGEIGIEZEN_GEIGERSENSOR_HPP
