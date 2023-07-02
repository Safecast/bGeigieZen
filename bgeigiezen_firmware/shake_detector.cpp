#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "shake_detector.h"

ShakeDetector::ShakeDetector()
    : Worker<bool>(),
      _gyroX(0),
      _gyroY(0),
      _gyroZ(0),
      _accX(0),
      _accY(0),
      _accZ(0),
      _pitch(0),
      _roll(0),
      _yaw(0),
      _temp(0) {
}

bool ShakeDetector::activate(bool retry) {
#ifdef M5_BASIC
  M5.IMU.Init();
#endif
  return true;
}

int8_t ShakeDetector::produce_data() {
#ifdef M5_BASIC
  M5.IMU.getGyroData(&_gyroX, &_gyroY, &_gyroZ);
  M5.IMU.getAccelData(&_accX, &_accY, &_accZ);
  M5.IMU.getAhrsData(&_pitch, &_roll, &_yaw);
  M5.IMU.getTempData(&_temp);
#endif
  return e_worker_idle;
}
