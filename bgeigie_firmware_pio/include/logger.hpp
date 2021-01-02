#include <M5Stack.h>

#include <config.hpp>
#include <cstdio>
#include <cstring>
#include <gps.hpp>
#include <hardwarecounter.hpp>
#include <sd_wrapper.hpp>

/* compute check sum of N bytes in array s */
char checksum(char *s, int N) {
  int i = 0;
  char chk = s[0];

  for (i = 1; i < N; i++) chk ^= s[i];

  return chk;
}

class BGeigieLogFormatter {
 private:
  // buffer to store one line of the log
  char _buffer[LOG_BUFFER_SIZE];

  int _device_id;
  bool _geiger_fresh = false, _gps_fresh = false;

  // Geiger stuff
  uint32_t _cpm, _cpb, _total_count;
  char _geiger_status;
  // GPS stuff
  int _year, _month, _day, _hour, _minute, _second;
  char _lat[10], _NS, _lon[11], _WE;
  float _altitude, _precision;
  int _satellites;
  char _gps_status;

 public:
  BGeigieLogFormatter(int device_id) : _device_id(device_id) {}

  void set_device_id(int device_id) { _device_id = device_id; }

  void feed(GPSSensor &gps) {
    auto &gps_data = gps.data();

    _year = gps_data.date.year();
    _month = gps_data.date.month();
    _day = gps_data.date.day();
    _hour = gps_data.time.hour();
    _minute = gps_data.time.minute();
    _second = gps_data.time.second();

    strcpy(_lat, gps_data.location.rawLatStr());
    _NS = gps_data.location.rawLat().negative ? 'N' : 'S';
    strcpy(_lon, gps_data.location.rawLngStr());
    _WE = gps_data.location.rawLng().negative ? 'W' : 'E';

    _altitude = gps_data.altitude.meters();
    _satellites =
        gps_data.satellites.isValid() ? gps_data.satellites.value() : 0;
    _precision = gps_data.hdop.isValid() ? gps_data.hdop.value() : 0.0;
    _gps_status = gps_data.location.isValid() ? 'A' : 'V';

    _gps_fresh = true;
  }

  void feed(const GeigerMeasurement &geiger_count) {
    _cpm = geiger_count.per_minute_raw();
    _cpb = geiger_count.per_bin();
    _total_count = geiger_count.total();
    _geiger_status = geiger_count.valid() ? 'A' : 'V';
    _geiger_fresh = true;
  }

  bool ready() { return _geiger_fresh && _gps_fresh; }

  const char *format() {
    size_t len = 0;
    std::memset(_buffer, 0, LOG_BUFFER_SIZE);
    std::sprintf(
        _buffer,
        "$%s,%04d,%02d-%02d-%02dT%02d:%02d:%02dZ,%ld,%ld,%ld,%c,%s,%c,%s,"
        "%c,%.1f,%c,%d,%.1f",
        DEVICE_HEADER, _device_id, _year, _month, _day, _hour, _minute, _second,
        (unsigned long)_cpm, (unsigned long)_cpb, (unsigned long)_total_count,
        _geiger_status, _lat, _NS, _lon, _WE, _altitude, _gps_status,
        _satellites, _precision);

    len = strlen(_buffer);
    _buffer[len] = '\0';

    // generate checksum
    uint8_t chk = checksum(_buffer + 1, len);

    // add checksum to end of line before sending
    if (chk < 16)
      sprintf(_buffer + len, "*0%X", (unsigned int)chk);
    else
      sprintf(_buffer + len, "*%X", (unsigned int)chk);

    // set the data to consummed
    _gps_fresh = false;
    _geiger_fresh = false;

    return _buffer;
  }
};

class SDLogger {
 private:
  const size_t filename_maxlen = 30;
  char _filename[30] = {0};

 public:
  SDLogger(const char *filename) {
    strncpy(_filename, filename, filename_maxlen);
  }

  sd_error_t log(const char *log_line) {
    auto my_file = SD.open(_filename, FILE_WRITE);

    if (my_file)
      my_file.println(log_line);
    else
      return SD_ERR_FILE_OPEN_W_FAIL;

    my_file.close();
  }
};
