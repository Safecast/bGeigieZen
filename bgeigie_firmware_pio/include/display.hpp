#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <TinyGPS++.h>

#include <config.hpp>
#include <setup.hpp>
#include <gps.hpp>
#include <geiger_counter.hpp>
#include <utility/M5Button.h>
#include <RBD_Timer.h>
#include <motion.hpp>
#include <menu.hpp>

// Dim then blank; tweak to taste (will become a menu setting)
constexpr uint8_t LEVEL_BRIGHT = 35;  // max brightness = 36
constexpr uint8_t LEVEL_DIMMED = 10;
constexpr uint8_t LEVEL_BLANKED = 0;
constexpr uint32_t DELAY_DIMMING_DEFAULT = 2*60*1000;  // ms before dimming screen
constexpr uint32_t DELAY_BLANKING_DEFAULT = 3*60*1000;  // ms before blanking screen

// printing routines
void printFloat(float val, bool valid, int len, int prec);
void printFloatFont(float val, bool valid, int len, int prec, int y, int x,
                    int font);
void printInt(unsigned long val, bool valid, int len);
void printIntFont(unsigned long val, bool valid, int len, int y, int x,
                  int font);
void printDate(TinyGPSDate &d);
void printTime(TinyGPSTime &t);

/*** Avoid name collision with M5Stack Core 2 UI ***/
namespace bGeigieZen {
  enum DisplayState {
    S_STARTUP,
    S_MAIN_DRAW,
    S_MAIN_SHOW,
    S_QRCODE_DRAW,
    S_QRCODE_SHOW,
    S_SURVEY_DRAW,
    S_SURVEY_SHOW,
    S_BLANKED,
    S_SURVEY_BLANKED,
    S_MENU_DRAW,
    S_MENU_SHOW
  };
}

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
  float battery_level = -1.0;

};

class InitState;
class MenuContext;

class Display {
 private:
  // size of the screen
  static const int width = 320;
  static const int height = 240;
  uint32_t _refresh_period_ms;

  // Dimming and blanking behaviour
  bool dimming_enabled = true;
  bool display_dimmed = false;
  uint32_t delay_dimming = DELAY_DIMMING_DEFAULT;  // ms before dimming
  uint32_t delay_blanking = DELAY_BLANKING_DEFAULT; // ms before blanking
  RBD::Timer timer_dimming{DELAY_DIMMING_DEFAULT};
  RBD::Timer timer_blanking{DELAY_BLANKING_DEFAULT};
  Button dimmingButton{265, 45, 55, 25, false, "DIMMING", {BLACK, TFT_ORANGE, TFT_ORANGE}};

  bGeigieZen::DisplayState state{bGeigieZen::S_STARTUP};
  DisplayData data;

  MotionDetect mpu{};

 public:
  Display(uint32_t refresh_period_ms)
      : _refresh_period_ms(refresh_period_ms), state(bGeigieZen::S_STARTUP) {
    timer_blanking.restart();
    timer_dimming.restart();
  }
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
  void draw_navbar_label(const char *s, const Button &b);
  void draw_main();
  void draw_qrcode();
  void draw_survey();
  void draw_menu();

  // Misc drawing routines
  void showDeviceId(uint32_t id, int16_t x = 10, int16_t y = 30);
  void showBatteryLevel(float level, int16_t x = 270, int16_t y = 30);

  // Misc set/get
  void set_dimblank_delays(uint32_t dimdelay, uint32_t blankdelay);
};


#endif  // __DISPLAY_H__
