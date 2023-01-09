#ifndef __GPS_HPP__
#define __GPS_HPP__

#include <M5Core2.h> // #include <M5Stack.h>
#include <TinyGPS++.h>
#include <config.hpp>

// We need a state machine to ensure that we always receive
// both GGA and RMC sentences before we consume the GPS data
// If we do not do that, there are rare cases where the GPS
// data is access between we receive the two sentences
// Since only RMC contains the date, it may happen that an
// incoherent state appear during the first update after midnight
// GGA updates the time to 00:00, but the date is still that of
// the previous day for a brief instant
// If an update during that time, the measurement will appear to be
// from 24 hours in the past
enum GPSState {
  GPS_Startup,
  GPS_Wait_GPGGA_Wait_GPRMC,
  GPS_Got_GPGGA_Wait_GPRMC,
  GPS_Got_GPRMC_Wait_GPGGA,
  GPS_Idle,
};

class GPSSensor {
 private:
  // The timeout when waiting for the second GPS sentence. The current value
  // of 500 ms assumes we are receiving updates at 1 s intervals.
  const uint32_t _timeout = 500;  // milliseconds

  HardwareSerial ss;
  TinyGPSPlus _data;

  GPSState _state = GPS_Startup;
  bool _available = false;
  uint32_t _last_update_millis = 0;

  bool _never_fixed = true;
  bool _has_fix = false;
  bool _time_valid = false;

  void _update_fix_metadata() {
    _has_fix = _data.location.isValid();
    if (_never_fixed)
        _never_fixed = false;

    _time_valid = _data.date.isValid() && _data.date.year() > GPS_INVALID_YEAR;
  }

 public:
  GPSSensor(int serial_number, uint32_t baud_rate) : ss(serial_number), _last_update_millis(millis()) {
    ss.begin(baud_rate);
  }

  uint32_t age_ms() const {
    uint32_t now = millis();
    if (now >= _last_update_millis)
      return now - _last_update_millis;
    else
      return ULONG_MAX + now - _last_update_millis;
  }

  bool has_started () const { return _state != GPS_Startup; }
  bool is_updating() const { return _state != GPS_Idle; }

  // Indicates fresh data is available
  bool available() const { return _available; }

  // The main routine processing GPS data
  void update();

  // Accessors for the GPS data
  bool has_fix() const { return _has_fix; }
  bool never_fixed() const { return _never_fixed; }
  bool time_valid() const { return _time_valid; }
  TinyGPSPlus &data() {
    _available = false;  // mark data as consumed
    return _data;
  }
  const TinyGPSPlus &data() const { return data(); }
};

#endif  // __GPS_HPP__
