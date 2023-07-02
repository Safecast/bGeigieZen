#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <M5Core2.h> // #include <M5Stack.h>

#include <config.hpp>
#include <cstdio>
#include <cstring>
#include <gps.hpp>
#include <geiger_counter.hpp>

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
  BGeigieLogFormatter(uint32_t device_id) : _device_id(device_id) {}
  BGeigieLogFormatter() : _device_id(SETUP_DEFAULT_DEVICE_ID) {}

  void set_device_id(uint32_t device_id) { _device_id = device_id; }

  void feed(GPSSensor &gps);
  void feed(const GeigerCounter &geiger_count);
  bool ready() { return _geiger_fresh && _gps_fresh; }

  const char *format();
};

class SDLogger {
 private:
  bool _folder_created = false;
  bool _logfile_created = false;
  bool _ready = false;
  static const size_t filename_maxlen = 32;
  char _filename[filename_maxlen] = {0};

 public:
  SDLogger() {};
  bool start(uint32_t device_id, uint16_t year, uint8_t month, uint8_t day);

  bool ready() const { return _folder_created && _logfile_created; }
  bool folder_created() const { return _folder_created; }

  bool log(const char *log_line) const;
};
#endif // __LOGGER_H__
