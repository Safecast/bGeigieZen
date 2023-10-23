#include "menu_window.h"
#include "controller.h"
#include "identifiers.h"
#include "workers/zen_button.h"
#include "drive_mode.h"
#include "survey_mode.h"
#include "fixed_mode.h"
#include "config_mode.h"

MenuWindow::MenuItem DRIVE_MENU_ITEM = {
    "Drive",
    "Put the zen on your car and drive!          ",
    "                                            ",
    true,
};
MenuWindow::MenuItem SURVEY_MENU_ITEM = {
    "Survey",
    "Take the Zen out of the case and test       ",
    "sources directly!                           ",
    true,
};
MenuWindow::MenuItem FIXED_MENU_ITEM = {
    "Fixed",
    "Place the zen at a fixed location and       ",
    "upload data over wifi!                      ",
    false,
};
MenuWindow::MenuItem LOG_VIEWER_MENU_ITEM = {
    "Logs",
    "View and upload logs!                       ",
    "                                            ",
    false,
};
MenuWindow::MenuItem SETTINGS_MENU_ITEM = {
    "Settings",
    "Configure your device!                      ",
    "                                            ",
    true,
};
MenuWindow::MenuItem ENTER_SIMPLE_MODE_MENU_ITEM = {
    "Simple mode",
    "Switch to simple mode!                      ",
    "                                            ",
    true,
};
MenuWindow::MenuItem ENTER_ADVANCED_MODE_MENU_ITEM = {
    "Advanced mode",
    "Switch to advanced mode!                    ",
    "                                            ",
    true,
};

MenuWindow::MenuWindow(): BaseScreen("Survey", true), menu_open(false), menu_index(0), advanced_menu{
    DRIVE_MENU_ITEM,
    SURVEY_MENU_ITEM,
    FIXED_MENU_ITEM,
    LOG_VIEWER_MENU_ITEM,
    SETTINGS_MENU_ITEM
} {

}

BaseScreen* MenuWindow::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  const auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  // Button 1 is move index down
  if (button1->is_fresh() && button1->get_data().shortPress) {
    menu_index++;
    menu_index %= 5;
    while (!advanced_menu[menu_index].enabled) {
      menu_index++;
      menu_index %= 5;
    }
  }
  if (button3->is_fresh() && button3->get_data().shortPress) {
    menu_open = false;
    return nullptr;
  }

  if (button2->is_fresh() && button2->get_data().shortPress) {
    // TODO: return selected screen
    switch (menu_index) {
      case 0:
        return DriveModeScreen::i();
      case 1:
        return SurveyModeScreen::i();
      case 2:
        return FixedModeScreen::i();
      case 3:
        return nullptr; //TODO
      case 4:
        return ConfigModeScreen::i();
    }
    return nullptr;
  }

  return nullptr;
}

void MenuWindow::render(const worker_map_t& workers, const handler_map_t& handlers) {
  if (!menu_open) {
    return;
  }

  // Draw buttons
  drawButton1("Next");
  drawButton2("Enter");
  drawButton3("Back", TFT_ORANGE);

  // Draw menu rect
  M5.Lcd.drawRoundRect(210, 20, 90, 100, 4, TFT_ORANGE);
  for (int i = 0; i < 5; ++i) {
    M5.Lcd.setTextColor(advanced_menu[i].enabled ? (i == menu_index ? TFT_ORANGE : TFT_WHITE) : TFT_DARKGREY, TFT_BLACK);
    M5.Lcd.drawString(advanced_menu[i].title, 230, 40 + (i * 10));
    if (i == menu_index) {
      M5.Lcd.drawString(">", 220, 40 + (i * 10));
    } else {
      M5.Lcd.drawString(" ", 220, 40 + (i * 10));
    }
  }

  // Draw tooltip bar
  M5.Lcd.drawRoundRect(20, 180, 280, 30, 4, TFT_ORANGE);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString(advanced_menu[menu_index].tooltip_l1, 30, 194);
  M5.Lcd.drawString(advanced_menu[menu_index].tooltip_l2, 30, 206);
}

bool MenuWindow::is_open() const {
  return menu_open;
}

void MenuWindow::enter_screen(Controller& controller) {
  menu_open = true;
}

void MenuWindow::leave_screen(Controller& controller) {
  menu_open = false;
}
