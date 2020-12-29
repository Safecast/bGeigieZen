#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <TinyGPS++.h>

#include <hardwarecounter.hpp>
#include <gps.hpp>

// printing routines
void printFloat(float val, bool valid, int len, int prec);
void printFloatFont(float val, bool valid, int len, int prec, int y, int x, int font);
void printInt(unsigned long val, bool valid, int len);
void printIntFont(unsigned long val, bool valid, int len, int y, int x, int font);
void printDate(TinyGPSDate &d);
void printTime(TinyGPSTime &t);

class Display {

  private:
    uint32_t _refresh_period_ms;
    bool _ready = false;

  public:
    Display(uint32_t refresh_period_ms)
      : _refresh_period_ms(refresh_period_ms) {}
    void setup();
    void update(const GeigerMeasurement &geiger_count);
    void update(GPSSensor &geiger_count);
};

#endif // __DISPLAY_H__
