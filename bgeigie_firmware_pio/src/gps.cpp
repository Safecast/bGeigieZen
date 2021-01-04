#include <gps.hpp>

void GPSSensor::update() {
  if ((_state == GPS_Startup || _state == GPS_Idle) && ss.available()) {
    _available = false;
    _state = GPS_Wait_GPGGA_Wait_GPRMC;
  } else if (!ss.available() && (_state == GPS_Got_GPRMC_Wait_GPGGA ||
                                 _state == GPS_Got_GPGGA_Wait_GPRMC)) {
    if (age_ms() >= _timeout) {
      _available = false;

      // We want to go back to idle state in case of time out,
      // except if we have never received data at all, which indicates
      // that the serial, or device is malfunctioning
      if (_state != GPS_Startup)
        _state = GPS_Idle;
    }
  }

  while (ss.available()) {
    bool is_valid_sentence = _data.encode(ss.read());

    if (is_valid_sentence) {
      switch (_state) {
        case GPS_Startup:
        case GPS_Idle:
        case GPS_Wait_GPGGA_Wait_GPRMC:
          if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPRMC)
            _state = GPS_Got_GPRMC_Wait_GPGGA;
          else if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPGGA)
            _state = GPS_Got_GPGGA_Wait_GPRMC;
          break;

        case GPS_Got_GPGGA_Wait_GPRMC:
        case GPS_Got_GPRMC_Wait_GPGGA:
          if (_data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPRMC ||
              _data.curSentenceType == TinyGPSPlus::GPS_SENTENCE_GPGGA) {
            _available = true;
            _state = GPS_Idle;
            _last_update_millis = millis();
            _update_fix_metadata();
          }
          break;
      }
    }
  }
}

