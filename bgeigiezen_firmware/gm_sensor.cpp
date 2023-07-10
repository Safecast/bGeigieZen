#include "gm_sensor.h"

GeigerMullerSensor::GeigerMullerSensor() : Worker<GMData>() {
}

bool GeigerMullerSensor::activate(bool retry) {
  // Connect to geigier muller sensor module connection
  return true;
}

int8_t GeigerMullerSensor::produce_data() {
  data.CPM = 30;
  data.usvh = 0.09;
  return e_worker_idle;
}
