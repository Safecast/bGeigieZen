#ifndef BGEIGIEZEN_BASE_SCREEN_H_
#define BGEIGIEZEN_BASE_SCREEN_H_

#include <Supervisor.hpp>

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

class BaseScreen {
 public:
  virtual BaseScreen* handle_input(const worker_map_t &workers) = 0;
  virtual void enter_screen();
  virtual void leave_screen();

  virtual void render(const worker_map_t &workers, const handler_map_t &handlers) = 0;
  virtual const char* get_title() const;

 protected:

  explicit BaseScreen(const char* title);
  virtual ~BaseScreen() = default;

  void drawButton(uint16_t x, const char *text, uint32_t border_color);
  void drawButton1(const char *text, uint32_t border_color = TFT_WHITE);
  void drawButton2(const char *text, uint32_t border_color = TFT_WHITE);
  void drawButton3(const char *text, uint32_t border_color = TFT_WHITE);

  char _title[20];
};

#endif //BGEIGIEZEN_BASE_SCREEN_H_
