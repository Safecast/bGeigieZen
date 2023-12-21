#ifndef ZEN_INFO_SCREEN_H
#define ZEN_INFO_SCREEN_H

#include "base_screen.h"

class ZenInfoScreen : public BaseScreen {
 public:
  static ZenInfoScreen* i() {
    static ZenInfoScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  enum ZenInfoPage {
    e_zen_info_page_main,
    e_zen_info_page_qr,
  };

  explicit ZenInfoScreen();

  ZenInfoPage page;

};

#endif //ZEN_INFO_SCREEN_H
