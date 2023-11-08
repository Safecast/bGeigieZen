#include "menu_window.h"
#include "config_mode.h"
#include "controller.h"
#include "debug_mode.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "identifiers.h"
#include "survey_mode.h"
#include "user_config.h"
#include "workers/zen_button.h"

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
MenuWindow::MenuItem DEBUG_MENU_ITEM = {
    "Debug info",
    "Connected modules, their data and status all",
    "together in a simple view                   ",
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

MenuWindow::MenuWindow() : BaseScreen("Menu", true), menu_open(false), menu_index(0), advanced_menu{
                                                                                          DRIVE_MENU_ITEM,
                                                                                          SURVEY_MENU_ITEM,
                                                                                          FIXED_MENU_ITEM,
                                                                                          LOG_VIEWER_MENU_ITEM,
                                                                                          SETTINGS_MENU_ITEM,
                                                                                          DEBUG_MENU_ITEM} {
}

BaseScreen* MenuWindow::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  const auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  // Button 1 is move index down
  if (button1->is_fresh() && button1->get_data().shortPress) {
    menu_index++;
    menu_index %= ADVANCED_MENU_ITEMS;
    while (!advanced_menu[menu_index].enabled) {
      menu_index++;
      menu_index %= ADVANCED_MENU_ITEMS;
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
      case 5:
        return DebugModeScreen::i();
    }
    return nullptr;
  }

  return nullptr;
}

void MenuWindow::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!menu_open) {
    return;
  }

  // Draw buttons
  drawButton1("Next");
  drawButton2("Enter");
  drawButton3("Back", e_button_active);

  // Draw menu rect
  M5.Lcd.drawRoundRect(210, 20, 90, 100, 4, LCD_COLOR_ACTIVE);
  for (int i = 0; i < ADVANCED_MENU_ITEMS; ++i) {
    M5.Lcd.setTextColor(advanced_menu[i].enabled ? (i == menu_index ? LCD_COLOR_ACTIVE : LCD_COLOR_DEFAULT) : TFT_DARKGREY, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString(advanced_menu[i].title, 230, 40 + (i * 10));
    if (i == menu_index) {
      M5.Lcd.drawString(">", 220, 40 + (i * 10));
    }
    else {
      M5.Lcd.drawString(" ", 220, 40 + (i * 10));
    }
  }

  // Draw tooltip bar
  M5.Lcd.drawRoundRect(20, 180, 280, 30, 4, LCD_COLOR_ACTIVE);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
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