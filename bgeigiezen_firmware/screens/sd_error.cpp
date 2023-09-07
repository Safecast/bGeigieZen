#include "sd_error.h"
#include "workers/zen_button.h"
#include "identifiers.h"
#include "menu_window.h"
#include "controller.h"
#include "handlers/local_storage.h"
#include "utils/device_utils.h"
#include "drive_mode.h"

SdErrorScreen::SdErrorScreen(): BaseScreen(""), error_type(SdErrorType::k_unknown) {

}

BaseScreen* SdErrorScreen::handle_input(const worker_map_t& workers) {
  const auto& controller = workers.worker<Controller>(k_worker_controller_state);
  const auto& button1 = workers.worker<ZenButton>(k_worker_button_1);
  const auto& button2 = workers.worker<ZenButton>(k_worker_button_2);
  const auto& button3 = workers.worker<ZenButton>(k_worker_button_3);

  switch (error_type) {
    case k_unknown:
      break;
    case k_no_sd_with_storage:
      if (button1->is_fresh() && button3->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DriveModeScreen::i();
      }
      break;
    case k_no_sd_no_storage:
      if (button1->is_fresh() && button3->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DriveModeScreen::i();
      }
      break;
    case k_empty_sd_with_storage:
      if (button1->is_fresh() && button3->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DriveModeScreen::i();
      }
      break;
    case k_empty_sd_no_storage:
      if (button1->is_fresh() && button3->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      break;
  }
  return nullptr;
}

void SdErrorScreen::render(const worker_map_t& workers, const handler_map_t& handlers) {
  if (error_type == k_unknown) {
    const auto& controller_data = workers.worker<Controller>(k_worker_controller_state)->get_data();
    const auto& storage = handlers.handler<LocalStorage>(k_handler_local_storage);
    switch (controller_data.sd_card_available) {
      case DeviceState::e_sd_unavailable:
        error_type = storage->get_device_id() ? k_no_sd_with_storage : k_no_sd_no_storage;
        break;
      case DeviceState::e_sd_no_conf:
      case DeviceState::e_sd_invalid_conf:
        error_type = storage->get_device_id() ? k_empty_sd_with_storage : k_empty_sd_no_storage;
        break;
      default:
        break;
    }
  }

  switch (error_type) {

    case k_unknown:
      break;
    case k_no_sd_with_storage:
      drawButton1("Reboot");
      drawButton3("Continue");
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Lcd.drawString("No SDCARD in slot", 50, 50, 4);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString("You can continue with local settings.", 5, 90, 2);
      M5.Lcd.drawString("Or insert an SD card and reboot.", 25, 90, 2);
      break;
    case k_no_sd_no_storage:
      drawButton1("Reboot");
      drawButton3("Continue");
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Lcd.drawString("No SDCARD in slot", 50, 50, 4);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString("You can continue in minimal mode.", 5, 90, 2);
      M5.Lcd.drawString("Or insert an SD card and reboot.", 25, 90, 2);
      break;
    case k_empty_sd_with_storage:
      drawButton1("Reboot");
      drawButton2("Write");
      drawButton3("Continue");
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Lcd.drawString("No valid config found on SD card", 50, 50, 4);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString("You can initialize the SD card from saved settings.", 5, 90, 2);
      M5.Lcd.drawString("Or continue in minimal mode.", 25, 90, 2);
      break;
    case k_empty_sd_no_storage:
      drawButton1("Reboot");
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Lcd.drawString("Unable to initialize device", 50, 50, 4);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString("Please insert an SD card with valid config to continue", 5, 90, 2);
      break;
  }

  //display Safecast copyright
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);

}

void SdErrorScreen::enter_screen() {
  error_type = SdErrorType::k_unknown;
}

void SdErrorScreen::leave_screen() {
  BaseScreen::leave_screen();
}