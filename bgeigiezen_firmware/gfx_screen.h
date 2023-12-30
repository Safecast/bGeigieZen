#ifndef BGEIGIEZEN_GFX_SCREEN_H_
#define BGEIGIEZEN_GFX_SCREEN_H_

#include "workers/local_storage.h"
#include "screens/menu_window.h"
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

  void handle_report(const worker_map_t &workers, const handler_map_t &handlers) override;
  void initialize() override;

private:
  void off();
  void clear();

  void setBrightness(uint8_t lvl, bool overdrive = false);

  Controller& _controller;
  LocalStorage& _settings;
  unsigned long _last_render;
  BaseScreen* _screen;
  MenuWindow* _menu;
};

#endif //BGEIGIEZEN_GFX_SCREEN_H_
