#include "threaded_gps.hpp"

/* convert long integer from TinyGPS to float WGS84 degrees */
float get_wgs84_coordinate(unsigned long val)
{
  double result = 0.0;
  result = val/10000000.0;
  result = ((result-(int)result)/60.0)*100 + (int)result;
  return (float)result;
}

void GPSThread::init()
{
  _parser = TinyGPS(true);

  // initialize status flags
  _ready = false;

  // start the thread
  int returnValue = pthread_create(&_the_thread, NULL, &GPSThread::update_loop, this);
 
  if (returnValue) {
    _serial_output->println("An error has occurred");
  }
}

bool GPSThread::prepare_data(GPSData &dest)
{
  if (_update_mutex.try_lock())
  {
    // cast the reference
    dest = _info;
    dest.fresh = true;
    _update_mutex.unlock();
    return true;
  }

  return false;
}

// Update GPS info from new sentence
void GPSThread::fill_info()
{
  // blocking wait
  _update_mutex.lock();

  // get GPS date
  _parser.crack_datetime(
      &(_info.year), 
      &(_info.month), 
      &(_info.day), 
      &(_info.hour), 
      &(_info.minute), 
      &(_info.second), 
      &(_info.hundredths), 
      &(_info.age));

  if (TinyGPS::GPS_INVALID_AGE == _info.age) 
  {
    _info.year = DEFAULT_YEAR, _info.month = 0, _info.day = 0, 
    _info.hour = 0, _info.minute = 0, _info.second = 0, 
    _info.hundredths = 0;
  }

  // get GPS position, altitude and speed
  _parser.get_position(&(_info.x), &(_info.y), &(_info.age));
  _info.gps_status = _parser.status();
  _info.faltitude = _parser.f_altitude();
  _info.fspeed = _parser.f_speed_kmph();
  _info.nbsat = _parser.satellites();
  _info.precision = _parser.hdop();

  _info.NS = 'N';
  _info.WE = 'E';
  if (_info.x < 0) { _info.NS = 'S'; _info.x = -_info.x;}
  if (_info.y < 0) { _info.WE = 'W'; _info.y = -_info.y;}

  // compute distance
  if (_parser.status()) {
    int trigger_dist = 25;
    float flat = get_wgs84_coordinate(_info.x);
    float flon = get_wgs84_coordinate(_info.y);

    if(_info.fspeed > 5)
      // fpspeed/3.6 * 5s = 6.94 m
      trigger_dist = 5;
    if(_info.fspeed > 10)
      trigger_dist = 10;
    if(_info.fspeed > 15)
      trigger_dist = 20;

    if(_gps_fix_first)
    {
      _gps_last_lat = flat;
      _gps_last_lon = flon;
      _gps_fix_first = false;
      _info.distance = 0;
    }
    else
    {
      // Distance in meters
      unsigned long int dist = (long int)TinyGPS::distance_between(flat, flon, _gps_last_lat, _gps_last_lon);

      if (dist > trigger_dist)
      {
        _info.distance += dist;
        _gps_last_lat = flat;
        _gps_last_lon = flon;
      }
    }
  }

  _update_mutex.unlock();
}

void *GPSThread::update_loop(void* param)
{
  GPSThread *gps_thread = (GPSThread *)param;

  // Start never ending loop
  for(;;) {

    while (gps_thread->serial_in().available())
    {
      if (gps_thread->available())
        gps_thread->unset_avalaible();

      char c = gps_thread->serial_in().read();

      if (c == '\n')  // update the info at then end of each line
      {
        // this function is blocking
        // (waiting on the data structure to be available)
        gps_thread->fill_info();
      }

      if (gps_thread->_parser.encode(c) || c == '\n') // Did a new valid sentence come in?
        gps_thread->set_avalaible();
    }
  }
}
