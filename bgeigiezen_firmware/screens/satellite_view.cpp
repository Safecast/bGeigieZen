#include "satellite_view.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/gps_connector.h"
#include "workers/navsat_collector.h"
#include "workers/zen_button.h"

#define COLOR_SAT_SIGNAL_STRONG TFT_WHITE
#define COLOR_SAT_SIGNAL_MEDIUM TFT_YELLOW
#define COLOR_SAT_SIGNAL_WEAK TFT_ORANGE
#define COLOR_SAT_USED_NAV TFT_BLUE
#define COLOR_SAT_HEALTHY TFT_RED
#define COLOR_SAT_UNKNOWN TFT_BLACK


const SatelliteViewScreen::MenuItem SATELLITE_MENU[SatelliteViewScreen::e_satellite_MENU_MAX] = {
    {.title="Satellites", .tooltip="View satellite constellation map", .enabled=true},
    {.title="Set default", .tooltip="Configure GPS module to use default satellites", .enabled=false},
    {.title="Set Americas", .tooltip="Configure GPS module to optimise use of satellites", .enabled=false},
    {.title="Set Europe/Africa", .tooltip="Configure GPS module to optimise use of satellites", .enabled=false},
    {.title="Set Asia/Oceania", .tooltip="Configure GPS module to optimise use of satellites", .enabled=false},
    {.title="Cold start", .tooltip="Send cold start signal to the GPX module", .enabled=true},
    {.title="Factory reset", .tooltip="Send factory reset signal to the GPS module", .enabled=false},
};

SatelliteViewScreen SatelliteViewScreen_i;

SatelliteViewScreen::SatelliteViewScreen() : BaseScreenWithMenu("gps map", true) {
  required_gps = true;
}

BaseScreen* SatelliteViewScreen::handle_input(Controller& controller, const worker_map_t& workers) {

  if (menu_open()) {
    return handle_menu_input(controller, workers, SATELLITE_MENU, e_satellite_MENU_MAX);
  }
  else {
    const auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    const auto navsat = workers.worker<NavsatCollector>(k_worker_navsat_collector);

    if (gps->active() && navsat->get_active_state() == NavsatCollector::e_state_inactive) {
      // reconnect navsat worker once gps worker is connected
      controller.set_worker_active(k_worker_navsat_collector, true);
      if (navsat->active()) {
        set_status_message(F(" Nav-sat connected, this can take a few seconds "));
      } else {
        set_status_message(F(" Nav-sat was unable to connect "));
      }
    }

    const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    const auto button3 = workers.worker<ZenButton>(k_worker_button_3);
    if (button1->is_fresh() && button1->get_data().shortPress) {
      open_menu(true);
      M5.Lcd.clear();
      force_next_render();
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // screen specific action
      switch (_current_page) {
        case e_satellite_page_main:
          controller.set_worker_active(k_worker_navsat_collector, false);
          controller.set_worker_active(k_worker_gps_connector, false);
          controller.set_worker_active(k_worker_gps_connector, true);
          controller.set_worker_active(k_worker_navsat_collector, true);
          force_next_render();
          break;
        default:
          break;
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  }

  return nullptr;
}

void SatelliteViewScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  ///

  if (menu_open()) {
    if (!force) {
      return;
    }
    return render_menu(SATELLITE_MENU, e_satellite_MENU_MAX);
  }

  const auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto navsat = workers.worker<NavsatCollector>(k_worker_navsat_collector);

  M5_LOGD("NAVSAT RENDER %d %d", navsat->active(), navsat->is_fresh());

  if (force || (navsat && navsat->is_fresh())) {

    int16_t  compAngle = 0;
    int16_t  mapRadius = 90;
    int16_t  mapSatRadius = 80; // Keep sat graphics inside map
    int16_t  mapCenterX = 108;
    int16_t  mapCenterY = 110;
    uint8_t  numSats = 0;
    int16_t  xCoord;
    int16_t  yCoord;
    int16_t  satRadius = 10;
    int32_t  satColor;
    int16_t  satRingRadius = 12;
    int32_t  satRingColor;
    int16_t  satAzimuth;
    int8_t   satElevation;
    char _dispStr[4];

    // Clear screen (later sprite support?)
    M5.Lcd.fillRect(mapCenterX - mapRadius, mapCenterY - mapRadius, mapRadius * 2, mapRadius * 2, LCD_COLOR_BACKGROUND);

    // Draw constellation map
    // draw main circles, one at 0deg, and one at 45deg elevation
    M5.Lcd.drawCircle(mapCenterX, mapCenterY, mapRadius, LCD_COLOR_DEFAULT);
    M5.Lcd.drawCircle(mapCenterX, mapCenterY, (mapRadius >> 1) + 1, LCD_COLOR_DEFAULT);

    // draw lines at 0, 45, 90, 135 etc degrees azimuth
    for (int16_t i = 0; i <= 7; i++) {
      xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * mapRadius);
      yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * mapRadius);
      M5.Lcd.drawLine(mapCenterX, mapCenterY, xCoord + mapCenterX, yCoord + mapCenterY, LCD_COLOR_DEFAULT);
      if(i % 2) continue;
      xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, 12, LCD_COLOR_BACKGROUND);
      char label;
      switch (i) {
        case 0: label = 'N'; break;
        case 2: label = 'E'; break;
        case 4: label = 'S'; break;
        case 6: label = 'W'; break;
      }
      M5.Lcd.drawChar(xCoord + mapCenterX - 5, yCoord + mapCenterY - 7, label, LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT, 2);
    }

    if (navsat->get_data().available) {

      // Set text size for satellite IDs
      const auto& navsat_info = navsat->get_data().navsat_info;

      // draw the positions of the sats
      for(int16_t i = navsat_info.numSvsEphValid -1; i >= 0; --i) {

        // Sat position
        ++numSats;
        satAzimuth = navsat_info.svSortList[i].azim;
        satElevation = navsat_info.svSortList[i].elev;
        if(satElevation < 0) satElevation = 0;
        xCoord = round(-sin(radians(satAzimuth + 180 + compAngle)) *
                       map(satElevation, 0, 90, mapSatRadius, 1));
        yCoord = round(cos(radians(satAzimuth + 180 + compAngle)) *
                       map(satElevation, 0, 90, mapSatRadius, 1));

        // Sat ring color based on SNR
        if(navsat_info.svSortList[i].cno >= 35) {
          satRingColor = COLOR_SAT_SIGNAL_STRONG;
        } else if(navsat_info.svSortList[i].cno >=20) {
          satRingColor = COLOR_SAT_SIGNAL_MEDIUM;
        } else {
          satRingColor = COLOR_SAT_SIGNAL_WEAK;
        }
        M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, satRingRadius, satRingColor);

        // Sat color based on svUsed
        if(navsat_info.svSortList[i].svUsed) {
          satColor = COLOR_SAT_USED_NAV;
        } else if(navsat_info.svSortList[i].healthy) {
          satColor = COLOR_SAT_HEALTHY;
        } else {
          satColor = COLOR_SAT_UNKNOWN;
        }
        M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, satRadius, satColor);


        // Sat label
        M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, satColor);
        sprintf(_dispStr, "%c%02d",
                navsat_info.svSortList[i].gnssIdType,
                navsat_info.svSortList[i].svId);
        M5.Lcd.drawString(_dispStr, xCoord + mapCenterX - 8, yCoord + mapCenterY + 5, &fonts::Font0);

      }
    }
  }

  if (force) {
    // Buttons
    drawButton1("Options");
    drawButton2("Reconnect");
    drawButton3("Menu");

    // Legend
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawCircle(215, 128 - 9, 5, COLOR_SAT_SIGNAL_STRONG);
    M5.Lcd.drawString("Strong signal", 225, 128, &fonts::Font2);
    M5.Lcd.drawCircle(215, 144 - 9, 5, COLOR_SAT_SIGNAL_MEDIUM);
    M5.Lcd.drawString("Medium signal", 225, 144, &fonts::Font2);
    M5.Lcd.drawCircle(215, 160 - 9, 5, COLOR_SAT_SIGNAL_WEAK);
    M5.Lcd.drawString("Weak signal", 225, 160, &fonts::Font2);
    M5.Lcd.fillCircle(215, 176 - 9, 5, COLOR_SAT_USED_NAV);
    M5.Lcd.drawString("Used in nav", 225, 176, &fonts::Font2);
    M5.Lcd.fillCircle(215, 192 - 9, 5, COLOR_SAT_HEALTHY);
    M5.Lcd.drawString("Tracked", 225, 192, &fonts::Font2);
  }
}

void SatelliteViewScreen::enter_screen(Controller& controller) {
  // This is done in handle input
//  controller.set_worker_active(k_worker_navsat_collector, true);
  switch (_current_page) {
    case e_satellite_page_cold_start:
      controller.get_gnss().coldStart();
      set_status_message(F(" GPS cold start, this can take a while..."));
      _current_page = e_satellite_page_main; // Back on main page
      open_menu(false);       // Re-enter menu
      break;
  }
}

void SatelliteViewScreen::leave_screen(Controller& controller) {
//  controller.set_worker_active(k_worker_navsat_collector, false);
}
