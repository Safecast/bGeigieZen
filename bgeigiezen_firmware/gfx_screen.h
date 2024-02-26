#ifndef BGEIGIEZEN_GFX_SCREEN_H_
#define BGEIGIEZEN_GFX_SCREEN_H_

#include "screens/menu_window.h"
#include "workers/local_storage.h"
#include <Arduino.h>
#include <Supervisor.hpp>
#include <screens/base_screen.h>

/**
 * M5 Screen renderer
 */
class GFXScreen : public Supervisor {
 public:
  explicit GFXScreen(LocalStorage& settings, Controller& controller);
  virtual ~GFXScreen() = default;

  void handle_report(const worker_map_t& workers, const handler_map_t& handlers) override;
  void initialize() override;

 private:
  enum ScreenStatus {
    e_screen_status_on,
    e_screen_status_dim,
    e_screen_status_off,
  };
  void set_screen_status(ScreenStatus status);
  void clear();

  void setBrightness(uint8_t lvl, bool overdrive = false);
  void render_screensaver();

  TFT_eSprite _saver;
  Controller& _controller;
  LocalStorage& _settings;
  unsigned long _last_render;
  uint32_t _last_interaction;
  bool _saver_enabled;
  int16_t _saver_x;
  int16_t _saver_y;
  int8_t _saver_x_direction;
  int8_t _saver_y_direction;
  ScreenStatus _screen_status;
  BaseScreen* _screen;
  MenuWindow* _menu;
};

#endif //BGEIGIEZEN_GFX_SCREEN_H_
