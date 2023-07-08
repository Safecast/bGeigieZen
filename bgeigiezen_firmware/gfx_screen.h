#ifndef BGEIGIEZEN_GFX_SCREEN_HPP
#define BGEIGIEZEN_GFX_SCREEN_HPP

#include <Arduino.h>
#include <Supervisor.hpp>


/**
 * M5 Screen renderer
 */
class GFXScreen : public Supervisor {
 public:

  explicit GFXScreen();
  virtual ~GFXScreen() = default;

  void handle_report(const worker_map_t& workers, const handler_map_t& handlers) override;

  void initialize() override;

 private:
  void off();
  void screenSplash();
  void screenDashboard();
  void screenSDError();
  void setBrightness(uint8_t lvl, bool overdrive = false);
  void clear();
};

#endif //BGEIGIEZEN_GFX_SCREEN_HPP
