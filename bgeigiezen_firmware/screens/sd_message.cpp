#include "sd_message.h"
#include "controller.h"
#include "default_entry_screen.h"
#include "drive_mode.h"
#include "first_time_startup.h"
#include "handlers/local_storage.h"
#include "identifiers.h"
#include "menu_window.h"
#include "utils/device_utils.h"
#include "workers/zen_button.h"

SdMessageScreen::SdMessageScreen() : BaseScreen("SD message", false), error_type(SdMessageType::k_unknown) {
}

BaseScreen* SdMessageScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto& button1 = workers.worker<ZenButton>(k_worker_button_1);
  const auto& button2 = workers.worker<ZenButton>(k_worker_button_2);
  const auto& button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (controller.get_data().sd_card_status == SDInterface::SdStatus::e_sd_config_status_ok) {
    return DefaultEntryScreen::i();
  }

  switch (error_type) {
    case k_unknown:
      if (button1->is_fresh() && button1->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      break;
    case k_new_device:
      return FirstTimeStartupScreen::i();
    case k_no_sd_with_storage:
      if (button1->is_fresh() && button1->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DefaultEntryScreen::i();
      }
      break;
    case k_empty_sd_no_storage:
    case k_no_sd_no_storage:
      if (button1->is_fresh() && button1->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DefaultEntryScreen::i();
      }
      break;
    case k_empty_sd_with_storage:
      if (button1->is_fresh() && button1->get_data().shortPress) {
        DeviceUtils::shutdown(true);
        return nullptr;
      }
      if (button2->is_fresh() && button2->get_data().shortPress) {
        controller.write_sd_config();
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DefaultEntryScreen::i();
      }
      break;
    case k_config_sd_different_id:
      if (button2->is_fresh() && button2->get_data().shortPress) {
        controller.load_sd_config();
      }
      if (button2->is_fresh() && button2->get_data().shortPress) {
        controller.write_sd_config();
      }
      if (button3->is_fresh() && button3->get_data().shortPress) {
        return DefaultEntryScreen::i();
      }
      break;
  }
  return nullptr;
}

void SdMessageScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  switch (error_type) {
    case k_unknown:
      drawButton1("Reboot");
      break;
    case k_no_sd_no_storage:
      drawButton1("Reboot");
      drawButton3("Continue");
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("Welcome to your Zen!", 40, 60, 4);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("Please insert your bGeigieZen SD-card", 10, 100, 2);
      M5.Lcd.drawString("into the SD-card slot and restart the device,", 10, 120, 2);
      M5.Lcd.drawString("or continue in minimal mode.", 10, 140, 2);
      M5.Lcd.drawString("For more info, visit bgeigiezen.safecast.jp", 10, 180, 2);
      break;
    case k_empty_sd_no_storage:
      drawButton1("Reboot");
      drawButton3("Continue");
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("Welcome to your Zen!", 40, 60, 4);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("Please add SAFEZEN.txt your SD-card,", 10, 100, 2);
      M5.Lcd.drawString("or continue in minimal mode.", 10, 120, 2);
      M5.Lcd.drawString("For more info, visit bgeigiezen.safecast.jp", 10, 160, 2);
      break;
    case k_no_sd_with_storage:
      drawButton1("Reboot");
      drawButton3("Continue");
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("SD card notification", 50, 50, 4);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("No SDCARD in slot", 10, 90, 2);
      M5.Lcd.drawString("You can continue in minimal mode.", 10, 110, 2);
      M5.Lcd.drawString("Or insert an SD card and reboot.", 10, 130, 2);
      break;
    case k_empty_sd_with_storage:
      drawButton1("Reboot");
      drawButton2("Write");
      drawButton3("Continue");
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("SD card notification", 50, 50, 4);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("No valid config found on SD card", 5, 90, 2);
      M5.Lcd.drawString("You can initialize the SD card from saved settings.", 5, 110, 2);
      M5.Lcd.drawString("Or continue in minimal mode.", 5, 130, 2);
      break;
    case k_config_sd_different_id:
      drawButton1("Load");
      drawButton2("Overwrite");
      drawButton3("Continue");
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("SD card notification", 50, 50, 4);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString("SD config with different device id found", 5, 90, 2);
      M5.Lcd.drawString("Press Load to use the SD card settings", 5, 110, 2);
      M5.Lcd.drawString("Press Overwrite to write device settings to the SD card", 5, 130, 2);
      M5.Lcd.drawString("Or press continue and ignore this message", 5, 150, 2);
      break;
    default:
      break;
  }

  //display Safecast copyright
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("2023", 285, 215, 1);
}

void SdMessageScreen::enter_screen(Controller& controller) {
  auto sd_status = controller.get_data().sd_card_status;
  auto local_available = controller.get_data().local_available;
  switch (sd_status) {
    case SDInterface::e_sd_config_status_not_ready:
      error_type = local_available ? SdMessageType::k_no_sd_with_storage : SdMessageType::k_no_sd_no_storage;
      break;
    case SDInterface::e_sd_config_status_ok:
      error_type = local_available ? SdMessageType::k_unknown : SdMessageType::k_new_device;
      break;
    case SDInterface::e_sd_config_status_config_different_id:
      error_type = SdMessageType::k_config_sd_different_id;
      break;
    case SDInterface::e_sd_config_status_config_no_content:
    case SDInterface::e_sd_config_status_no_config_file:
      error_type = local_available ? SdMessageType::k_empty_sd_with_storage : SdMessageType::k_empty_sd_no_storage;
      break;
  }
}

void SdMessageScreen::leave_screen(Controller& controller) {
}