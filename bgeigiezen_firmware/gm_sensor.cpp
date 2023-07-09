#include "gm_sensor.h"

GeigerMullerSensor::GeigerMullerSensor() : Worker<uint32_t>() {
}

bool GeigerMullerSensor::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  return true;
}

int8_t GeigerMullerSensor::produce_data() {
  data = 30;
  return e_worker_idle;
}
