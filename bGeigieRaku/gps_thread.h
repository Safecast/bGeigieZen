/*
 * Class implementing threaded parsing of the GPS data
 *
 * This class uses the Serial1
 *
 * 2018, Robin Scheibler
 */
#ifndef __GPS_THREAD__
#define __GPS_THREAD__

#include <Arduino.h>
#include <pthread.h>
#include "TinyGPS.h"

#define GPS_AVAILABLE 'A'  // indicates gps data are ready (available)
#define GPS_VOID      'V'  // indicates gps data not ready (void)
#define DEFAULT_YEAR 2013

typedef struct
{
  int year = DEFAULT_YEAR;
  byte month = 0, day = 0, hour = 0, minute = 0, second = 0, hundredths = 0;
  long int x = 0, y = 0;
  float faltitude = 0, fspeed = 0;
  unsigned short nbsat = 0;
  unsigned long precision = 0;
  unsigned long age;
  char NS = 'N';
  char WE = 'E';
  char gps_status = GPS_VOID;
  unsigned long int distance = 0;
}
gps_info_t;


class GPSThread
{
  public:
    // status flag to indicate if the GPS is information is ready
    bool ready = false;
    bool locked = false;

    // For the distance calculation
    bool gps_fix_first = true;
    float gps_last_lon = 0, gps_last_lat = 0;
    gps_info_t info;

    TinyGPS parser;
    pthread_t theThread;

    void begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin);
    void fillInfo();
    bool lock();
    void unlock();

    // GPS status
    bool isReady() { return ready; };

    // These two functions lock update of the GPS parameters
    static void *updateLoop(void *);
};

#endif
