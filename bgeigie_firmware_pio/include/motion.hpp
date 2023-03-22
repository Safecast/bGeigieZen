#ifndef __MOTION_H__
#define __MOTION_H__

#include <M5Core2.h>
#include <utility/MPU6886.h>

/// @brief Use MPU6886 IMU to detect motion for wake-on-motion
class MotionDetect {
  private:
    MPU6886 mpu{};
    int32_t ACCEL_THRESHOLD = 1500;  // adjust to taste
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    int16_t prev_ax = 0;
    int16_t prev_ay = 0;
    int16_t prev_az = 0;
    int32_t sum_deltas = 0;
    bool detected = false;
  public:
    void update();  // read the IMU
    int32_t motion();  // Sum of abs() of component deltas
    bool motionDetected();  // true if motion > threshold
    int32_t getAccelThreshold();
    void setAccelThreshold(const int32_t threshold);

};

#endif  // __MOTION_H__
