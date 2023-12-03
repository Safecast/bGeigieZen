#ifndef BGEIGIEZEN_BASE_SCREEN_H_
#define BGEIGIEZEN_BASE_SCREEN_H_

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "controller.h"
#include <Supervisor.hpp>

class BaseScreen {
 public:
  enum ButtonState {
    e_button_default,
    e_button_active,
    e_button_disabled
  };

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) = 0;
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
  virtual void do_render(const worker_map_t& workers, const handler_map_t& handlers) final;

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void force_next_render() final;

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

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void render(const worker_map_t& workers, const handler_map_t& handlers, bool force);

  void drawButton1(const char* text, ButtonState state = e_button_default) const;
  void drawButton2(const char* text, ButtonState state = e_button_default) const;
  void drawButton3(const char* text, ButtonState state = e_button_default) const;

  int printFloatFont(float val, int prec, int x, int y, int font) const;
  int printIntFont(unsigned long val, int x, int y, int font) const;

 private:
  void drawButton(uint16_t x, const char* text, ButtonState state) const;

  char _title[20];
  bool _status_bar;
  bool _force_render;
};

#endif //BGEIGIEZEN_BASE_SCREEN_H_
