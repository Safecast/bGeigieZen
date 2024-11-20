#include "fixed_mode.h"
#include "handlers/api_connector.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "workers/gm_sensor.h"
#include "workers/log_aggregator.h"
#include "workers/zen_button.h"

FixedModeScreen FixedModeScreen_i;

FixedModeScreen::FixedModeScreen() : BaseScreen("Real-time", true) {
  required_gps = true;
  required_tube = true;
  required_wifi = true;
}

BaseScreen* FixedModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void FixedModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton3("Menu");

  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  const auto& api_connector = handlers.handler<ApiConnector>(k_handler_api_reporter);


  if (gm_sensor->is_fresh() || force) {
    if (settings->get_cpm_usvh()) {
      // Display CPM big, usvh small
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 0, 100, &fonts::Font7);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh, 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cpm_width, 52, 220 - cpm_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      cpm_width += M5.Lcd.drawString(" CPM", cpm_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(cpm_width, 74, 220 - cpm_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" uSv/h   ", 0 + ush_width, 140, &fonts::Font4); // Prints after ush value
    } else {
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh, 3, 0, 100, &fonts::Font7);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 220 - ush_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      ush_width += M5.Lcd.drawString(" uSv/h", ush_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(ush_width, 74, 220 - ush_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" CPM   ", 0 + cpm_width, 140, &fonts::Font4); // Prints after ush value
    }
  }

  if (api_connector->get_status() == ApiConnector::e_handler_processing) {
    set_status_message(F(" SENDING... "));
  }

  if (api_connector->get_status() == ApiConnector::e_api_reporter_send_success) {
    set_status_message(F(" SENT DATA TO API! "));
  }


  // Display GPS data always, change colour if not fresh
  if (log_aggregator->is_fresh() || force) {
    const auto& log_data = log_aggregator->get_data();
    // Print drive data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);

    // Print fixed setting data
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Home latitude  :");
    M5.Lcd.printf("%0.6f  ", settings->get_fixed_latitude());
    M5.Lcd.setCursor(0, 159);
    M5.Lcd.print("Home longitude :");
    M5.Lcd.printf("%0.6f  ", settings->get_fixed_longitude());
    M5.Lcd.setCursor(0, 168);
    M5.Lcd.print("You are        :");

    if (log_data.gps_valid) {
      M5.Lcd.setTextColor(log_data.in_fixed_range ? LCD_COLOR_DEFAULT : LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf(log_data.in_fixed_range ? "At home    " : "Roaming    ");
    } else {
      M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("Without GPS");
    }


    M5.Lcd.setCursor(0, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("total uploads  :%d", api_connector->get_post_count());

    // Print location data
    M5.Lcd.setCursor(170, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Latitude   :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", log_data.latitude);
    M5.Lcd.setCursor(170, 159);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Longitude  :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", log_data.longitude);
    M5.Lcd.setCursor(170, 168);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Altitude   :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f    ", log_data.altitude);
  }

  if (force) {
    // Display QR on the side
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    char qr_url[130];
    const auto& config = workers.worker<LocalStorage>(k_worker_local_storage);
    sprintf(qr_url, FIXED_MODE_GRAFANA_URL, config->get_fixed_device_id(), config->get_fixed_device_id());
    M5.Lcd.qrcode(qr_url, 220, 40, 90);
  }

}

void FixedModeScreen::enter_screen(Controller& controller) {
  controller.set_handler_active(k_handler_api_reporter, true);
}

void FixedModeScreen::leave_screen(Controller& controller) {
  controller.set_handler_active(k_handler_api_reporter, false);
}