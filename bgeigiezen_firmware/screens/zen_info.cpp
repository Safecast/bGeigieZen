#include "zen_info.h"
#include "menu_window.h"
#include "identifiers.h"
#include "user_config.h"
#include "workers/zen_button.h"

ZenInfoScreen::ZenInfoScreen() : BaseScreen("Info", true), page(e_zen_info_page_main) {
}

BaseScreen* ZenInfoScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto qr_button = workers.worker<ZenButton>(k_worker_button_1);
  if (qr_button->is_fresh() && qr_button->get_data().shortPress) {
    page = page == e_zen_info_page_main ? e_zen_info_page_qr : e_zen_info_page_main;
    force_next_render();
  }
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void ZenInfoScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  drawButton3("Menu");

  M5.Lcd.fillRect(0, 20, 320, 180, LCD_COLOR_BACKGROUND);

  switch (page) {
    case e_zen_info_page_main:
      drawButton1("Website");
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("At some point there will be a", 10, 46, 2);
      M5.Lcd.drawString("bunch of info about your Zen here!", 10, 62, 2);
      break;
    case e_zen_info_page_qr:
      drawButton1("Back");
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("Visit us at", 127, 36, 1);
      M5.Lcd.qrcode("https://bgeigiezen.safecast.jp", 95, 40, 130);
      M5.Lcd.drawString("bgeigiezen.safecast.jp", 93, 186, 2);
      break;
  }

}

void ZenInfoScreen::enter_screen(Controller& controller) {
  page = e_zen_info_page_main;
  force_next_render();
}