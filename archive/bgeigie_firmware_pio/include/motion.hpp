#ifndef __MOTION_H__
#define __MOTION_H__

#include <M5Core2.h>
#include <utility/MPU6886.h>

/* Accel. and gyro. thresholds based on MPU6886.h full scale defaults:
 * Gyscale = GFS_2000DPS; (2000 degrees per second)
 * Acscale = AFS_8G; (8G)
 * Adjust for practical values.
 */
constexpr int32_t ACCEL_THRESHOLD_DEFAULT = 2000;
constexpr int32_t GYRO_THRESHOLD_DEFAULT = 800;

/// @brief Use MPU6886 IMU to detect motion for wake-on-motion
class MotionDetect {
  private:
    int32_t accel_threshold = ACCEL_THRESHOLD_DEFAULT;
    int32_t gyro_threshold = GYRO_THRESHOLD_DEFAULT;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    int16_t prev_ax = 0;
    int16_t prev_ay = 0;
    int16_t prev_az = 0;
    int16_t gx = 0;
    int16_t gy = 0;
    int16_t gz = 0;
    int16_t prev_gx = 0;
    int16_t prev_gy = 0;
    int16_t prev_gz = 0;
    int32_t sum_accel_deltas = 0;
    int32_t sum_gyro_deltas = 0;
    bool detected = false;
  public:
    MPU6886 mpu{};  // public, available for other applications
    void update();  // read the IMU, update internal variables
    int32_t motion();  // Sum of abs() of component deltas (accel + gyro)
    bool motionDetected();  // true if motion > thresholds
    int32_t getAccelThreshold();
    void setAccelThreshold(const int32_t threshold);
    int32_t getGyroThreshold();
    void setGyroThreshold(const int32_t threshold);

};

#endif  // __MOTION_H__
