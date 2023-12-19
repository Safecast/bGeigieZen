#include "debug_mode.h"
#include "debugger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"

DebugModeScreen::DebugModeScreen() : BaseScreen("Debug", true) {
}

BaseScreen* DebugModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void DebugModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  ///
  // Display something
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
  const auto& rtc = workers.worker<RtcConnector>(k_worker_rtc_connector);
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);

  drawButton1("");
  drawButton2("");
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 25);
  M5.Lcd.printf("Battery: %d%% %s\n",
                battery->get_data().percentage,
                battery->get_data().isCharging ? "(charging)" : "          ");
  if (gm_sensor->active()) {
    M5.Lcd.printf("Geiger counter %s\n"
                  " CPM: raw %d, comp %d %s       \n   uSv/h: %.4f,  Bq/m2: %.0f   \n"
                  " CP5S: %d,  uSv/h: %.4f,  Bq/m2: %.0f      \n",
                  gm_sensor->get_data().valid ? "(valid)             " : "(collecting data...)",
                  gm_sensor->get_data().cpm_raw,
                  gm_sensor->get_data().cpm_comp,
                  gm_sensor->get_data().alert ? "A" : " ",
                  gm_sensor->get_data().uSv,
                  gm_sensor->get_data().Bqm2,
                  gm_sensor->get_data().cp5s,
                  gm_sensor->get_data().uSv_5sec,
                  gm_sensor->get_data().Bqm2_5sec);
  }
  else {
    M5.Lcd.printf("Geiger counter\n Module not available\n\n");
  }

  if (gps->active()) {
    M5.Lcd.printf("GPS %s\n"
                  " location: %s                \n"
                  "  lat, long, altitude (MSL), HDOP:\n"
                  "  %.5f, %.5f, %.2f, %.1f \n"
                  " satellites: %d tracked, %d in view   \n"
                  " date: %04d-%02d-%02d %s     \n"
                  " time: %02d:%02d:%02d %s     \n",
                  gps->get_data().valid() ? "(valid)        " : "(incomplete...)",
                  gps->get_data().location_valid ? "             " : "(unavailable)",
                  gps->get_data().latitude,
                  gps->get_data().longitude,
                  gps->get_data().altitudeMSL,
                  gps->get_data().pdop,
                  gps->get_data().satsTracked,
                  gps->get_data().satsInView,
                  gps->get_data().date_valid ? gps->get_data().year : 0,
                  gps->get_data().date_valid ? gps->get_data().month : 0,
                  gps->get_data().date_valid ? gps->get_data().day : 0,
                  gps->get_data().date_valid ? "             " : "(unavailable)",
                  gps->get_data().time_valid ? gps->get_data().hour : 0,
                  gps->get_data().time_valid ? gps->get_data().minute : 0,
                  gps->get_data().time_valid ? gps->get_data().second : 0,
                  gps->get_data().time_valid ? "             " : "(unavailable)");
  }
  else {
    M5.Lcd.printf("GPS\n Module not available\n\n");
  }

  if (rtc->active()) {
    M5.Lcd.printf("RTC\n"
                  " VoltLow: %s\n"
                  " date: %04d-%02d-%02d time: %02d:%02d:%02d\n",
                  rtc->get_data().low_voltage ? "Low voltage " : "Voltage good",
                  rtc->get_data().year,
                  rtc->get_data().month,
                  rtc->get_data().day,
                  rtc->get_data().hour,
                  rtc->get_data().minute,
                  rtc->get_data().second);
  }
  else {
    M5.Lcd.printf("RTC\n Module not available\n\n");
  }

  M5.Lcd.printf("Logging data\n"
                " Log string: %s\n",
                log_aggregator->get_data().log_string);
}
