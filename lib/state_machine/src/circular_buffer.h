#ifndef STATE_MACHINE_CIRCULAR_BUFFER_HPP
#define STATE_MACHINE_CIRCULAR_BUFFER_HPP

#include <stdint.h>

/**
 * Simple circular buffer
 * @tparam T: Type of the buffer
 * @tparam max: max items in buffer
 */
template<typename T, uint16_t max>
class CircularBuffer {
 public:
  CircularBuffer() : buffer(), count(0), current(0) {};
  virtual ~CircularBuffer() = default;

  /**
   * Get the next value from the buffer, should check if buffer is not empty before calling this
   * @return: next value T
   */
  const T& get() {
    T& val = buffer[current];
    --count;
    ++current;
    current %= max;
    return val;
  };

  /**
   * Add a value to the buffer. If the buffer is full, it will throw away the oldest value and add this.
   * @param val
   */
  void add(const T& val) {
    buffer[(current + count) % max] = val;
    if(count < max) {
      ++count;
    } else {
      ++current;
      current %= max;
    }
  };

  /**
   * Check if the buffer is empty
   * @return: true if buffer is empty
   */
  bool empty() const {
    return count == 0;
  }

  /**
   * Get the amount of items in the buffer
   * @return
   */
  uint16_t get_count() const {
    return count;
  }

  /**
   * Clear the buffer
   */
  void clear() {
    count = 0;
  }

 private:
  T buffer[max];
  uint16_t count;
  uint16_t current;
};

#endif //STATE_MACHINE_CIRCULAR_BUFFER_HPP
