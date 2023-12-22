#include "menu_window.h"
#include "config_mode.h"
#include "controller.h"
#include "debug_mode.h"
#include "debugger.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "identifiers.h"
#include "survey_mode.h"
#include "user_config.h"
#include "workers/zen_button.h"
#include "zen_info.h"

MenuWindow::MenuItem DRIVE_MENU_ITEM = {
    "Drive mode",
    "Put the zen on your car and drive!",
    true,
    &DriveModeScreen_i,
};
MenuWindow::MenuItem SURVEY_MENU_ITEM = {
    "Survey mode",
    "Take the Zen out of the case and test   sources directly!",
    true,
    &SurveyModeScreen_i,
};
MenuWindow::MenuItem FIXED_MENU_ITEM = {
    "Fixed mode",
    "Place the zen at a    fixed location and    upload data over     wifi!",
    false,
    &FixedModeScreen_i,
};
MenuWindow::MenuItem LOG_VIEWER_MENU_ITEM = {
    "Log viewer",
    "View and upload     logs over wifi!",
    false,
    nullptr,
};
MenuWindow::MenuItem SETTINGS_MENU_ITEM = {
    "Settings",
    "Configure your      device!",
    true,
    &ConfigModeScreen_i,
};
MenuWindow::MenuItem MORE_INFO_MENU_ITEM = {
    "Learn more",
    "Explore what you    can do with your     bGeigieZen",
    true,
    &ZenInfoScreen_i,
};
MenuWindow::MenuItem DEBUG_MENU_ITEM = {
    "Debug info",
    "Connected modules, their data and status all in a simple view",
    true,
    &DebugModeScreen_i,
};
MenuWindow::MenuItem ENTER_SIMPLE_MODE_MENU_ITEM = {
    "Simple mode",
    "Switch to simple mode!",
    true,
};
MenuWindow::MenuItem ENTER_ADVANCED_MODE_MENU_ITEM = {
    "Advanced mode",
    "Switch to advanced mode!",
    true,
};

MenuWindow MenuWindow_i;

MenuWindow::MenuWindow() : BaseScreen("Menu", true),
                           menu_open(false),
                           menu_index(0),
                           selected_screen_index(0),
                           advanced_menu{
                               DRIVE_MENU_ITEM,
                               SURVEY_MENU_ITEM,
                               FIXED_MENU_ITEM,
                               LOG_VIEWER_MENU_ITEM,
                               SETTINGS_MENU_ITEM,
                               MORE_INFO_MENU_ITEM,
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
    force_next_render();
  }

  if (button3->is_fresh() && button3->get_data().shortPress) {
    menu_open = false;
    return nullptr;
  }

  if (button2->is_fresh() && button2->get_data().shortPress && advanced_menu[menu_index].enabled) {
    selected_screen_index = menu_index;
    return advanced_menu[menu_index].screen;
  }

  return nullptr;
}

void MenuWindow::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!menu_open || !force) {
    return;
  }

  // Draw buttons
  drawButton1("Next");
  drawButton2("Enter", advanced_menu[menu_index].enabled ? e_button_default : e_button_disabled);
  drawButton3("Close", e_button_active);

  // Draw menu overlay border
  M5.Lcd.drawRoundRect(10, 20, 300, 170, 4, LCD_COLOR_STALE_INCOMPLETE);
  // Visually connect button to overlay
#ifdef M5_CORE2
  M5.Lcd.fillRect(220, 12, 90, 12, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawLine(220, 12, 220, 20, LCD_COLOR_STALE_INCOMPLETE);
  M5.Lcd.drawLine(309, 12, 309, 23, LCD_COLOR_STALE_INCOMPLETE);
#elif M5_BASIC
  M5.Lcd.fillRect(210, 12, 90, 12, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawLine(210, 12, 210, 20, LCD_COLOR_STALE_INCOMPLETE);
  M5.Lcd.drawLine(299, 12, 299, 20, LCD_COLOR_STALE_INCOMPLETE);
#endif
  // Draw tooltip block
  M5.Lcd.fillRoundRect(161, 26, 142, 158, 4, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawLine(160, 33, 160, 177, LCD_COLOR_STALE_INCOMPLETE);
//  M5.Lcd.drawRoundRect(160, 25, 144, 160, 4, LCD_COLOR_INACTIVE);


  for (int i = 0; i < ADVANCED_MENU_ITEMS; ++i) {
    M5.Lcd.setTextColor(advanced_menu[i].enabled ? (i == menu_index ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_DEFAULT) : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawLine(16, 48 + (i * 16), 159, 48 + (i * 16), (i == menu_index ? (advanced_menu[i].enabled ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_INACTIVE) : LCD_COLOR_BACKGROUND));
    M5.Lcd.setCursor(16, 40 + (i * 16), 2);
    if (i == menu_index) {
      M5.Lcd.print("> ");
    }
    else {
      M5.Lcd.print("  ");
    }
    M5.Lcd.print(advanced_menu[i].title);
    M5.Lcd.print("  ");
  }

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(170, 30);
  M5.Lcd.println(advanced_menu[menu_index].title);
  size_t tooltip_length = strlen(advanced_menu[menu_index].tooltip);
  uint8_t line = 0;
  uint8_t current_line_length = 0;
  M5.Lcd.setCursor(170, 60);
  for (size_t i = 0; i < tooltip_length; ++i) {
    if (current_line_length == 0 && advanced_menu[menu_index].tooltip[i] == ' ') {
      continue;
    }
    current_line_length += M5.Lcd.drawChar(advanced_menu[menu_index].tooltip[i], 170 + current_line_length, 60 + (line * 16), 2);
    if (current_line_length > 124) {
      line += 1;
      current_line_length = 0;
    }
  }
  if (!advanced_menu[menu_index].enabled) {
    M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString("Not available", 170, 110 + (line * 16), 2);
  }

  M5.Lcd.setCursor(0, 0, 1);
}

bool MenuWindow::is_open() const {
  return menu_open;
}

void MenuWindow::enter_screen(Controller& controller) {
  menu_open = true;
  menu_index = selected_screen_index;
  force_next_render();
}

void MenuWindow::leave_screen(Controller& controller) {
  menu_open = false;
}
