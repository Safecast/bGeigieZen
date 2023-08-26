#ifndef BGEIGIEZEN_IMU_SENSOR_H_
#define BGEIGIEZEN_IMU_SENSOR_H_

#include <Arduino.h>
#include <Worker.hpp>

/**
 * Uses the Inertial Measurement Unit in M5 device to detect any shaking (wake up)
 */
class ShakeDetector : public Worker<bool> {
 public:

  explicit ShakeDetector();

  virtual ~ShakeDetector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  // IMU sensor data
  float _gyroX;
  float _gyroY;
  float _gyroZ;
  float _accX;
  float _accY;
  float _accZ;
  float _pitch;
  float _roll;
  float _yaw;
  float _temp;
};

#endif //BGEIGIEZEN_IMU_SENSOR_H_
