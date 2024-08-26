#include "zen_info.h"
#include "menu_window.h"
#include "identifiers.h"
#include "user_config.h"
#include "workers/zen_button.h"

ZenInfoScreen ZenInfoScreen_i;

ZenInfoScreen::ZenInfoScreen() : BaseScreen("Info", true), _page(e_zen_info_page_main) {
}

BaseScreen* ZenInfoScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto qr_button = workers.worker<ZenButton>(k_worker_button_1);
  if (qr_button->is_fresh() && qr_button->get_data().shortPress) {
    _page = _page == e_zen_info_page_main ? e_zen_info_page_qr : e_zen_info_page_main;
    force_next_render();
  }
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void ZenInfoScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  drawButton3("Menu");

  M5.Lcd.fillRect(0, 20, 320, 180, LCD_COLOR_BACKGROUND);

  switch (_page) {
    case e_zen_info_page_main:
      return render_page_main(workers, handlers);
    case e_zen_info_page_qr:
      return render_page_qr(workers, handlers);
  }
}

void ZenInfoScreen::render_page_main(const worker_map_t& workers, const handler_map_t& handlers) {
  // Temp only display Zen qr code on main page
  return render_page_qr(workers, handlers);
  drawButton1("Website");
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("At some point there will be a", 10, 46, &fonts::Font2);
  M5.Lcd.drawString("bunch of info about your Zen here!", 10, 62, &fonts::Font2);
}

void ZenInfoScreen::render_page_qr(const worker_map_t& workers, const handler_map_t& handlers) {
//  drawButton1("Back");
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("Visit us at", 127, 36, &fonts::Font0);
  M5.Lcd.qrcode("https://bgeigiezen.safecast.jp", 95, 40, 130);
  M5.Lcd.drawString("bgeigiezen.safecast.jp", 93, 186, &fonts::Font2);
}

void ZenInfoScreen::enter_screen(Controller& controller) {
  _page = e_zen_info_page_main;
  force_next_render();
}