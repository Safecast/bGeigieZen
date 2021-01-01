#ifndef __GPS_HPP__
#define __GPS_HPP__

#include <M5Stack.h>
#include <TinyGPS++.h>

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

    GPSState _state = GPS_Idle;
    bool _available = false;
    uint32_t _last_update_millis = 0;

  public:
    GPSSensor(int serial_number, uint32_t baud_rate)
      : ss(serial_number) { ss.begin(baud_rate); }

    uint32_t millis_since_last_update() const {
      uint32_t now = millis();
      if (now >= _last_update_millis)
        return now - _last_update_millis;
      else
        return ULONG_MAX + now - _last_update_millis;
    }

    bool is_updating() const {
      return _state != GPS_Idle;
    }

    bool available() const {
      return _available;
    }

    void update() {

      if (_state == GPS_Idle && ss.available()) {
        _available = false;
        _state = GPS_Wait_GPGGA_Wait_GPRMC;
      }
      else if (!ss.available()
          && (_state == GPS_Got_GPRMC_Wait_GPGGA || _state == GPS_Got_GPGGA_Wait_GPRMC)) {

        if (millis_since_last_update() >= _timeout) {
          _available = false;
          _state = GPS_Idle;
        }
      }

      while (ss.available()) {
        bool is_valid_sentence = _data.encode(ss.read());

        if (is_valid_sentence) {
          switch (_state) {
            case GPS_Idle:
            case GPS_Wait_GPGGA_Wait_GPRMC:
              if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPRMC)
                _state = GPS_Got_GPRMC_Wait_GPGGA;
              else if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPGGA)
                _state = GPS_Got_GPGGA_Wait_GPRMC;
              break;

            case GPS_Got_GPGGA_Wait_GPRMC:
            case GPS_Got_GPRMC_Wait_GPGGA:
              if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPRMC
                  || _data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPGGA) {
                _available = true;
                _state = GPS_Idle;
                _last_update_millis = millis();
              }
              break;
          }
        }
      }
    }

    TinyGPSPlus &data() {
      _available = false;  // mark data as consumed
      return _data;
    }
    const TinyGPSPlus &data() const {
      return data();
    }
};

#endif // __GPS_HPP__
