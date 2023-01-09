#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <TinyGPS++.h>

#include <config.hpp>
#include <setup.hpp>
#include <gps.hpp>
#include <geiger_counter.hpp>

// printing routines
void printFloat(float val, bool valid, int len, int prec);
void printFloatFont(float val, bool valid, int len, int prec, int y, int x,
                    int font);
void printInt(unsigned long val, bool valid, int len);
void printIntFont(unsigned long val, bool valid, int len, int y, int x,
                  int font);
void printDate(TinyGPSDate &d);
void printTime(TinyGPSTime &t);

enum DisplayState2 {
  S_STARTUP,
  S_MAIN_DRAW,
  S_MAIN_SHOW,
  S_QRCODE_DRAW,
  S_QRCODE_SHOW
};

struct DisplayData {
  // device setup info
  uint32_t device_id;

  // Geiger
  bool geiger_fresh = true;
  bool geiger_valid = false;
  uint32_t geiger_cpm = 0;
  float geiger_uSv = 0.0;

  // GPS
  bool gps_fresh = true;
  TinyGPSInteger gps_satellites;
  TinyGPSAltitude gps_altitude;
  TinyGPSSpeed gps_speed;
  TinyGPSCourse gps_course;
  TinyGPSLocation gps_location;
  TinyGPSTime gps_time;
  TinyGPSDate gps_date;

  // Battery
  int8_t battery_level = -1;
};

class Display {
 private:
  // size of the screen
  static const int width = 320;
  static const int height = 240;
  uint32_t _refresh_period_ms;

  DisplayState2 state{S_STARTUP};
  DisplayData data;

 public:
  Display(uint32_t refresh_period_ms)
      : _refresh_period_ms(refresh_period_ms), state(S_STARTUP) {}
  void clear();
  void feed(const GeigerCounter &geiger_count);
  void feed(GPSSensor &geiger_count);
  void feed(const Setup &device_setup) {
    data.device_id = device_setup.config().device_id;
  }
  void feed_battery_level(int8_t level) { data.battery_level = level; }

  // Routines that runs the state machine
  void update();

  // Routines to draw the different screens
  void draw_base();
  void draw_navbar(const char *A, const char *B, const char *C);
  void draw_main();
  void draw_qrcode();
};

#endif  // __DISPLAY_H__
