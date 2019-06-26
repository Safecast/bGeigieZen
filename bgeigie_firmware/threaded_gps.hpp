/*
 * Class implementing threaded parsing of the GPS data
 *
 * This class uses the Serial1
 *
 * 2018, Robin Scheibler
 */
#ifndef __GPS_THREAD__
#define __GPS_THREAD__

#include <pthread.h>
#include <cstdint>
#include <mutex>
#include "TinyGPS.h"

#include "serial_source.hpp"
#include "measurements.hpp"

#define DEFAULT_YEAR 2013

struct GPSData : public MeasurementData
{
  int year = DEFAULT_YEAR;
  uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 0, hundredths = 0;
  long int x = 0, y = 0;
  float faltitude = 0, fspeed = 0;
  unsigned short nbsat = 0;
  unsigned long precision = 0;
  unsigned long age;
  char NS = 'N';
  char WE = 'E';
  bool gps_status = false;
  unsigned long int distance = 0;
};

class GPSThread : public Measurement<GPSData>
{
  private:
    // status flag to indicate if the GPS is information is ready
    bool _ready = false;
    std::mutex _update_mutex;
    TinyGPS _parser;

    // For the distance calculation
    bool _gps_fix_first = true;
    float _gps_last_lon = 0, _gps_last_lat = 0;

    pthread_t _the_thread;

    GPSData _info;

    // This will fill the data structure from the TinyGPS parser
    void fill_info();

    // Function running the GPS update thread
    static void *update_loop(void *);

    // This the serial source of the GPS data
    SerialSource *_serial_input;
    SerialSink *_serial_output;

  public:

    GPSThread(SerialSource *serial_in, SerialSink *serial_out)
      : _serial_input(serial_in), _serial_output(serial_out) {}

    // Implementation of Measurement API
    void init();
    bool prepare_data(GPSData &dest);
};

#endif
