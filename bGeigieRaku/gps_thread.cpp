#include "gps_thread.h"

//HardwareSerial Serial1(1);

/* convert long integer from TinyGPS to float WGS84 degrees */
float get_wgs84_coordinate(unsigned long val)
{
  double result = 0.0;
  result = val/10000000.0;
  result = ((result-(int)result)/60.0)*100 + (int)result;
  return (float)result;
}

void GPSThread::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin)
{
  Serial1.begin(baud, config, rxPin, txPin);
  parser = TinyGPS(true);

  // initialize status flags
  ready = false;

  // start the thread
  int returnValue = pthread_create(&theThread, NULL, &GPSThread::updateLoop, this);
 
  if (returnValue) {
    Serial.println("An error has occurred");
  }
}

bool GPSThread::lock()
{
  if (!locked)
    locked = true;
  return locked;
}

void GPSThread::unlock()
{
  locked = false;
}

// Update GPS info from new sentence
void GPSThread::fillInfo()
{
  while (!lock())
    ;

  // get GPS date
  parser.crack_datetime(
      &(info.year), 
      &(info.month), 
      &(info.day), 
      &(info.hour), 
      &(info.minute), 
      &(info.second), 
      &(info.hundredths), 
      &(info.age));

  if (TinyGPS::GPS_INVALID_AGE == info.age) 
  {
    info.year = DEFAULT_YEAR, info.month = 0, info.day = 0, 
    info.hour = 0, info.minute = 0, info.second = 0, 
    info.hundredths = 0;
  }

  // get GPS position, altitude and speed
  parser.get_position(&(info.x), &(info.y), &(info.age));
  if (!parser.status()) {
    info.gps_status = GPS_VOID;
  } else {
    info.gps_status = GPS_AVAILABLE;
  }
  info.faltitude = parser.f_altitude();
  info.fspeed = parser.f_speed_kmph();
  info.nbsat = parser.satellites();
  info.precision = parser.hdop();

  info.NS = 'N';
  info.WE = 'E';
  if (info.x < 0) { info.NS = 'S'; info.x = -info.x;}
  if (info.y < 0) { info.WE = 'W'; info.y = -info.y;}

  // compute distance
  if (parser.status()) {
    int trigger_dist = 25;
    float flat = get_wgs84_coordinate(info.x);
    float flon = get_wgs84_coordinate(info.y);

    if(info.fspeed > 5)
      // fpspeed/3.6 * 5s = 6.94 m
      trigger_dist = 5;
    if(info.fspeed > 10)
      trigger_dist = 10;
    if(info.fspeed > 15)
      trigger_dist = 20;

    if(gps_fix_first)
    {
      gps_last_lat = flat;
      gps_last_lon = flon;
      gps_fix_first = false;
      info.distance = 0;
    }
    else
    {
      // Distance in meters
      unsigned long int dist = (long int)TinyGPS::distance_between(flat, flon, gps_last_lat, gps_last_lon);

      if (dist > trigger_dist)
      {
        info.distance += dist;
        gps_last_lat = flat;
        gps_last_lon = flon;
      }
    }
  }

  unlock();
}

void *GPSThread::updateLoop(void* param)
{
  GPSThread *gps_thread = (GPSThread *)param;

  // Start never ending loop
  for(;;) {

    while (Serial1.available())
    {
      if (gps_thread->ready)
        gps_thread->ready = false;

      char c = Serial1.read();

      if (c == '\n')  // update the info at then end of each line
      {
        while (!gps_thread->lock())
          ;
        gps_thread->fillInfo();
        gps_thread->unlock();
      }

      if (gps_thread->parser.encode(c) || c == '\n') // Did a new valid sentence come in?
        gps_thread->ready = true;
    }
  }
}

