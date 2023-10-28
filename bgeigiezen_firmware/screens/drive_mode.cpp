#include "drive_mode.h"
#include "debugger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"

DriveModeScreen::DriveModeScreen(): BaseScreen("Drive", true), _log_available(false), _log_to_file("") {
}

BaseScreen* DriveModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  _log_available = controller.get_data().sd_card_status == SDInterface::e_sd_config_status_ok;
  // TODO: handle log button

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void DriveModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers) {
  ///
  // Display something
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
  const auto& rtc = workers.worker<RtcConnector>(k_worker_rtc_connector);

  if (strlen(_log_to_file)) {
    // Is logging
    drawButton1("Stop log");
  } else {
    drawButton1("Start log", _log_available ? e_button_default : e_button_disabled);
  }
  drawButton2("");
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("Battery: %d%% %s\n",
                battery->get_data().percentage,
                battery->get_data().isCharging ? "(charging)" : "          ");
  if (gm_sensor->get_active_state() == GeigerCounter::e_state_active) {
    M5.Lcd.printf("Geiger counter %s\n"
                  " CPM raw: %d        \n"
                  " CPM comp: %d %s     \n   uSv/h: %.4f      \n   Bq/m2: %.0f       \n"
                  " CPB: %d      \n   uSv/h: %.4f      \n   Bq/m2: %.0f       \n",
                  gm_sensor->get_data().valid ? "(valid)             " : "(collecting data...)",
                  gm_sensor->get_data().cpm_raw,
                  gm_sensor->get_data().cpm_comp,
                  gm_sensor->get_data().alert ? "(ALERT!!!)" : "          ",
                  gm_sensor->get_data().uSv,
                  gm_sensor->get_data().Bqm2,
                  gm_sensor->get_data().cps,
                  gm_sensor->get_data().uSv_sec,
                  gm_sensor->get_data().Bqm2_sec);
  } else {
    M5.Lcd.printf("Geiger counter\n Module not available\n\n");
  }

  if (gps->get_active_state() == GpsConnector::e_state_active) {
    M5.Lcd.printf("GPS\n"
                  " location: %s                \n"
                  "  latitude: %.5f            \n"
                  "  longitude: %.5f           \n"
                  "  altitude: %.2f (MSL)      \n"
                  "  HDOP: %d  %s              \n"
                  " satellites: %d %s           \n"
                  " date: %04d-%02d-%02d %s     \n"
                  " time: %02d:%02d:%02d %s     \n",
                  gps->get_data().location_valid ? "             " : "(unavailable)",
                  gps->get_data().latitude * 1e-7,
                  gps->get_data().longitude * 1e-7,
                  gps->get_data().altitudeMSL * 1e-3,
                  gps->get_data().hdop,
                  gps->get_data().location_valid ? "             " : "(unavailable)",
                  gps->get_data().satsInView,
                  gps->get_data().satellites_valid ? "             " : "(unavailable)",
                  gps->get_data().date_valid ? gps->get_data().year : 0,
                  gps->get_data().date_valid ? gps->get_data().month : 0,
                  gps->get_data().date_valid ? gps->get_data().day : 0,
                  gps->get_data().date_valid ? "             " : "(unavailable)",
                  gps->get_data().time_valid ? gps->get_data().hour : 0,
                  gps->get_data().time_valid ? gps->get_data().minute : 0,
                  gps->get_data().time_valid ? gps->get_data().second : 0,
                  gps->get_data().time_valid ? "             " : "(unavailable)");
  } else {
    M5.Lcd.printf("GPS\n Module not available\n\n");
  }

  if (rtc->get_active_state() == RtcConnector::e_state_active) {
    M5.Lcd.printf("RTC\n"
                  " VoltLow: %s\n"
                  " date: %04d-%02d-%02d\n"
                  " time: %02d:%02d:%02d\n",
                  rtc->get_data().rtc_low_voltage ? "Low voltage " : "Voltage good",
                  rtc->get_data().year,
                  rtc->get_data().month,
                  rtc->get_data().day,
                  rtc->get_data().hour,
                  rtc->get_data().minute,
                  rtc->get_data().second
                  );
    } else {
    M5.Lcd.printf("RTC\n Module not available\n\n");
  }
}

void DriveModeScreen::leave_screen(Controller& controller) {
  // TODO: close logging to file
}
