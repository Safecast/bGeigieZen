#include <M5Core2.h> // #include <M5Stack.h>

#include <logger.hpp>

/* compute check sum of N bytes in array s */
char checksum(char *s, int N) {
  int i = 0;
  char chk = s[0];

  for (i = 1; i < N; i++) chk ^= s[i];

  return chk;
}

void BGeigieLogFormatter::feed(const GeigerCounter &geiger_count) {
  _cpm = geiger_count.per_minute_raw();
  _cpb = geiger_count.per_bin();
  _total_count = geiger_count.total();
  _geiger_status = geiger_count.valid() ? 'A' : 'V';
  _geiger_fresh = true;
}

void BGeigieLogFormatter::feed(GPSSensor &gps) {
  auto &gps_data = gps.data();

  _year = gps_data.date.year();
  _month = gps_data.date.month();
  _day = gps_data.date.day();
  _hour = gps_data.time.hour();
  _minute = gps_data.time.minute();
  _second = gps_data.time.second();

  strcpy(_lat, gps_data.location.rawLatStr());
  _NS = gps_data.location.rawLat().negative ? 'S' : 'N';
  strcpy(_lon, gps_data.location.rawLngStr());
  _WE = gps_data.location.rawLng().negative ? 'W' : 'E';

  _altitude = gps_data.altitude.meters();
  _satellites = gps_data.satellites.isValid() ? gps_data.satellites.value() : 0;
  _precision = gps_data.hdop.isValid() ? gps_data.hdop.value() : 0.0;
  _gps_status = gps_data.location.isValid() ? 'A' : 'V';

  _gps_fresh = true;
}

const char *BGeigieLogFormatter::format() {
  size_t len = 0;
  std::memset(_buffer, 0, LOG_BUFFER_SIZE);
  std::sprintf(
      _buffer,
      "$%s,%04d,%02d-%02d-%02dT%02d:%02d:%02dZ,%ld,%ld,%ld,%c,%s,%c,%s,"
      "%c,%.1f,%c,%d,%.1f",
      DEVICE_HEADER, _device_id, _year, _month, _day, _hour, _minute, _second,
      (unsigned long)_cpm, (unsigned long)_cpb, (unsigned long)_total_count,
      _geiger_status, _lat, _NS, _lon, _WE, _altitude, _gps_status, _satellites,
      _precision);

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

bool SDLogger::start(uint32_t device_id, uint16_t year, uint8_t month,
                     uint8_t day) {
  // "YYYY/BBBBMMDD.LOG", 18 charachters, including terminating NULL
  // Y: year, M: month, D: day, B: device id
  if (year > 9999) year = 9999;
  if (day > 31) day = 31;
  if (month > 12)
    month = 12;
  else if (month == 0)
    month = 1;
  if (device_id > 9999) device_id = 9999;

  sprintf(_filename, "/%04d/%04d%02d%02d.log", year, device_id, month, day);
  Serial.print("The filename ");
  Serial.println(_filename);

  // Create the sub-folder
  char folder[6];
  sprintf(folder, "/%04d", year);
  if (SD.exists(folder))
    _folder_created = true;
  else
    _folder_created = SD.mkdir(folder);
  if (!_folder_created) {
    Serial.print("Failed to create ");
    Serial.println(folder);
  }

  // Open the file and print the firmware version
  if (_folder_created) {
    char header_l2[9 + 4 + 1 + 4 + 1 + 4 + + 3 + 1];
    uint16_t maj = MAJOR > 9999 ? 9999 : MAJOR;
    uint16_t min = MINOR > 9999 ? 9999 : MINOR;
    uint16_t pat = PATCH > 9999 ? 9999 : PATCH;
    sprintf(header_l2, "%s%d.%d.%d%s", LOG_HEADER_LINE2, maj, min, pat, device_nickname);
    _logfile_created = log(LOG_HEADER_LINE1);
    _logfile_created = log(header_l2);
    _logfile_created = log(LOG_HEADER_LINE3);
  }

  return ready();
}

bool SDLogger::log(const char *log_line) const {
  auto my_file = SD.open(_filename, FILE_APPEND);  // opens in append mode

  if (!my_file) return false;

  my_file.println(log_line);
  my_file.close();
  return true;
}
