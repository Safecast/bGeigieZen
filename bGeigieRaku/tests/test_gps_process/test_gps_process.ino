#include "gps_thread.h"

GPSThread gps;
#define GPS_INTERVAL 1000

// Gps data buffers
#define BUFFER_SZ 12
static char lat[BUFFER_SZ];
static char lon[BUFFER_SZ];

/* convert long integer from TinyGPS to string "xxxxx.xxxx" */
void get_coordinate_string(bool is_latitude, unsigned long val, char *buf)
{
  unsigned long left = 0;
  unsigned long right = 0;

  left = val/100000;
  right = (val - left*100000)/10;
  if (is_latitude) {
    sprintf(buf, ("%04ld.%04ld"), left, right);
  } else {
    sprintf(buf, ("%05ld.%04ld"), left, right);
  }
}

void setup()
{

  Serial.begin(57600);

  // Start the GPS Thread
  gps.begin(9600, SERIAL_8N1, 33, 32);
}

void loop()
{

  // Lock the gps to avoid update of the info
  while (gps.lock())
    ;

  // Print some GPS information
  Serial.printf("year: %d, month: %d, day: %d\n", gps.info.year, (int)gps.info.month, (int)gps.info.day);
  Serial.printf("lat:%ld lon:%ld\n", gps.info.x, gps.info.y);
  get_coordinate_string(true, gps.info.x, lat);
  get_coordinate_string(false, gps.info.y, lon);
  Serial.printf("after processing: lat:%s lon:%s\n", lat, lon);

  // Unlock the GPS
  gps.unlock();

  Serial.printf("\n");

  delay(1000);
}
