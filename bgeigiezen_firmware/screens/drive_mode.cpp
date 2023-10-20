#include "drive_mode.h"
#include "workers/zen_button.h"
#include "workers/battery_indicator.h"
#include "workers/gps_connector.h"
#include "workers/gm_sensor.h"
#include "identifiers.h"
#include "menu_window.h"
#include "debugger.h"

#ifdef M5_CORE2
#include "workers/rtc_connector.h"
#endif

DriveModeScreen::DriveModeScreen(): BaseScreen("Drive") {
}

BaseScreen* DriveModeScreen::handle_input(const worker_map_t& workers) {
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
#ifdef M5_CORE2
  const auto& rtc = workers.worker<RtcConnector>(k_worker_rtc_connector);
#endif

  drawButton1("Start log");
  drawButton2("");
  drawButton3("Menu");

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("Battery: %d%% %s\n",
                battery->get_data().percentage,
                battery->get_data().isCharging ? "(charging)" : "          ");
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
                gps->get_data().year,
                gps->get_data().month,
                gps->get_data().day,
                gps->get_data().date_valid ? "             " : "(unavailable)",
                gps->get_data().hour,
                gps->get_data().minute,
                gps->get_data().second,
                gps->get_data().time_valid ? "             " : "(unavailable)");

/** @todo Remove this when initialization handles time setup properly. */
#ifdef M5_CORE2
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
#endif
}

void DriveModeScreen::leave_screen() {

}
