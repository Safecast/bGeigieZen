#ifndef BGEIGIEZEN_BASE_SCREEN_H_
#define BGEIGIEZEN_BASE_SCREEN_H_

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include <Supervisor.hpp>
#include "controller.h"

class BaseScreen {
 public:
  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t &workers) = 0;
  /**
   * Enter the screen, use controller to enable screen specific workers/handlers
   * @param controller
   */
  virtual void enter_screen(Controller& controller);

  /**
   * Leave the screen, use controller to disable screen specific workers/handlers
   * @param controller
   */
  virtual void leave_screen(Controller& controller);

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void render(const worker_map_t &workers, const handler_map_t &handlers) = 0;

  /**
   * Get screen name
   * @return
   */
  virtual const char* get_title() const;

  /**
   * Get screen name
   * @return
   */
  virtual bool has_status_bar() const;

 protected:

  explicit BaseScreen(const char* title, bool status_bar);
  virtual ~BaseScreen() = default;

  void drawButton1(const char *text, uint32_t border_color = TFT_WHITE);
  void drawButton2(const char *text, uint32_t border_color = TFT_WHITE);
  void drawButton3(const char *text, uint32_t border_color = TFT_WHITE);

 private:

  void drawButton(uint16_t x, const char *text, uint32_t border_color);

  char _title[20];
  bool _status_bar;
};

#endif //BGEIGIEZEN_BASE_SCREEN_H_
