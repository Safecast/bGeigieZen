#include "sd_wipe.h"
#include "controller.h"
#include "identifiers.h"
#include "menu_window.h"
#include "utils/sd_wrapper.h"
#include "utils/error_beep.h"
#include "workers/zen_button.h"

SDWipeScreen SDWipeScreen_i;

SDWipeScreen::SDWipeScreen() : BaseScreen("SD Card Wipe", true), _state(CONFIRM), _wipe_success(false) {
  required_tube = false;
  required_gps = false;
  required_wifi = false;
  required_sd = true;
}

BaseScreen* SDWipeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto btn_a = workers.worker<ZenButton>(k_worker_button_1);
  auto btn_c = workers.worker<ZenButton>(k_worker_button_3);

  if (btn_c->is_fresh() && btn_c->get_data().shortPress) {
    // Cancel/Back button pressed
    return &MenuWindow_i;
  }

  if (_state == CONFIRM && btn_a->is_fresh() && btn_a->get_data().shortPress) {
    // Confirm button pressed, start wiping
    _state = WIPING;
    force_next_render();
    
    // Perform the wipe operation
    _wipe_success = SDInterface::i().clear_all_logs();
    
    // Update state to complete
    _state = COMPLETE;
    force_next_render();
  }

  return nullptr;
}

void SDWipeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  M5.Lcd.fillRect(0, 40, 320, 180, LCD_COLOR_BACKGROUND);

  switch (_state) {
    case CONFIRM:
      setErrorColorWithBeep(workers);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(20, 60);
      M5.Lcd.println("WARNING: This will delete ALL data from the SD card!");
      M5.Lcd.setCursor(20, 80);
      M5.Lcd.println("All log files will be permanently deleted.");
      M5.Lcd.setCursor(20, 100);
      M5.Lcd.println("Your device settings will be preserved.");
      M5.Lcd.setCursor(20, 130);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.println("Are you sure you want to continue?");
      
      drawButton1("Confirm", e_button_active);
      drawButton2("");
      drawButton3("Cancel");
      break;
      
    case WIPING:
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(20, 80);
      M5.Lcd.println("Wiping SD card...");
      M5.Lcd.setCursor(20, 100);
      M5.Lcd.println("Please wait, this may take a moment.");
      
      drawButton1("");
      drawButton2("");
      drawButton3("");
      break;
      
    case COMPLETE:
      if (_wipe_success) {
        M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(20, 80);
        M5.Lcd.println("SD card wiped successfully!");
        M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        M5.Lcd.setCursor(20, 100);
        M5.Lcd.println("All log files have been deleted.");
      } else {
        setErrorColorWithBeep(workers);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(20, 80);
        M5.Lcd.println("Failed to wipe SD card!");
        M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        M5.Lcd.setCursor(20, 100);
        M5.Lcd.println("Please check if the SD card is properly inserted.");
      }
      
      drawButton1("");
      drawButton2("");
      drawButton3("Back to Menu");
      break;
  }
}

void SDWipeScreen::enter_screen(Controller& controller) {
  _state = CONFIRM;
  _wipe_success = false;
  set_status_message(F(" SD CARD WIPE "));
  force_next_render();
}

void SDWipeScreen::leave_screen(Controller& controller) {
  // Nothing to do when leaving
}
