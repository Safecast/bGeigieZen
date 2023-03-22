#include <motion.hpp>

    /// @brief Force a re-read of the IMU, update motion
    void MotionDetect::update() {
      mpu.getAccelAdc(&ax, &ay, &az);
      sum_deltas = abs(ax - prev_ax) + abs(ay - prev_ay) + abs(az - prev_az);
      detected = sum_deltas > ACCEL_THRESHOLD;
      prev_ax = ax;
      prev_ay = ay;
      prev_az = az;
    }

    /// @brief Sum of abs() of component deltas 
    /// @return int32_t sum
    int32_t MotionDetect::motion() {
      return sum_deltas;
    }

    /// @brief One-shot check that motion exceeded threshold since last update. 
    /// @return true if motion > threshold, false otherwise
    bool MotionDetect::motionDetected() {
      update();
      bool temp = detected;
      detected = false; 
      return temp;
    }

    /// @brief Return the current motion detection threshold
    /// @return int32_t ACCEL_THRESHOLD
    int32_t MotionDetect::getAccelThreshold() {
      return ACCEL_THRESHOLD;
    }

    /// @brief Set motion detection threshold
    /// @param threshold 
    void MotionDetect::setAccelThreshold(const int32_t threshold) {
        ACCEL_THRESHOLD = threshold;
    }
