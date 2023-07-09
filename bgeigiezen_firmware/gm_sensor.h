#ifndef BGEIGIEZEN_GEIGERSENSOR_HPP
#define BGEIGIEZEN_GEIGERSENSOR_HPP

#include <Worker.hpp>

/**
 * Geiger counter worker, produces CPM.
 */
class GeigerMullerSensor : public Worker<uint32_t> {
 public:

  explicit GeigerMullerSensor();

  virtual ~GeigerMullerSensor() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:

};

#endif //BGEIGIEZEN_GEIGERSENSOR_HPP
