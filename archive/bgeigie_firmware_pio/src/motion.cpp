#include <motion.hpp>

    /// @brief Force a re-read of the IMU, update motion
    void MotionDetect::update() {
      mpu.getAccelAdc(&ax, &ay, &az);
      mpu.getGyroAdc(&gx, &gy, &gz);
      sum_accel_deltas = abs(ax - prev_ax) + abs(ay - prev_ay) + abs(az - prev_az);
      sum_gyro_deltas = abs(gx - prev_gx) + abs(gy - prev_gy) + abs(gz - prev_gz);
      detected = (sum_accel_deltas > accel_threshold) || (sum_gyro_deltas > gyro_threshold);
      prev_ax = ax;
      prev_ay = ay;
      prev_az = az;
      prev_gx = gx;
      prev_gy = gy;
      prev_gz = gz;
    }

    /// @brief Sum of abs() of component deltas 
    /// @return int32_t sum
    int32_t MotionDetect::motion() {
      return sum_accel_deltas + sum_gyro_deltas;
    }

    /// @brief One-shot check that motion exceeded threshold since last update. 
    /// @return true if motion > threshold, false otherwise
    bool MotionDetect::motionDetected() {
      update();
      bool temp = detected;
      detected = false; 
      return temp;
    }

    /// @brief Return the current acceleration detection threshold
    /// @return int32_t threshold
    int32_t MotionDetect::getAccelThreshold() {
      return accel_threshold;
    }

    /// @brief Set motion detection threshold
    /// @param threshold 
    void MotionDetect::setAccelThreshold(const int32_t threshold) {
        accel_threshold = threshold;
    }

    /// @brief Return the current motion detection threshold
    /// @return int32_t gyro_threshold
    int32_t MotionDetect::getGyroThreshold() {
      return gyro_threshold;
    }

    /// @brief Set motion detection threshold
    /// @param threshold 
    void MotionDetect::setGyroThreshold(const int32_t threshold) {
        gyro_threshold = threshold;
    }
