#include "satellite_view.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/gps_connector.h"
#include "workers/navsat_collector.h"
#include "workers/zen_button.h"

SatelliteViewScreen SatelliteViewScreen_i;

SatelliteViewScreen::SatelliteViewScreen() : BaseScreen("Satellites", true) {
  required_gps = true;
}

BaseScreen* SatelliteViewScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto cold_start_button = workers.worker<ZenButton>(k_worker_button_1);
  const auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  if (gps->active() && cold_start_button->is_fresh() && cold_start_button->get_data().shortPress) {
    controller.gps_cold_start();
    set_status_message(F(" GPS cold start, this can take a while..."));
    return nullptr;
  }
  const auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void SatelliteViewScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  ///
  const auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto navsat = workers.worker<NavsatCollector>(k_worker_navsat_collector);


  drawButton1("Cold start", gps->active() ? e_button_default : e_button_disabled);
  drawButton2("");
  drawButton3("Menu");

  if (force || (navsat && navsat->is_fresh()) || gps->is_fresh()) {
    // Draw constellation map
    
    int16_t  compAngle = 0;
    int16_t  mapRadius = 90;
    int16_t  mapSatRadius = 80; // Keep sat graphics inside map
    int16_t  mapCenterX = 160;
    int16_t  mapCenterY = 110;
    uint8_t  numSats = 0;
    int16_t  xCoord;
    int16_t  yCoord;
    int16_t  satRadius = 8;
    uint32_t satColor;
    int16_t  satRingRadius = 10;
    uint32_t satRingColor;
    int16_t  satAzimuth;
    int8_t   satElevation;
    char _dispStr[4];
    
    // draw main circles, one at 0deg, and one at 45deg elevation
    M5.Lcd.drawCircle(mapCenterX, mapCenterY, mapRadius, WHITE);
    M5.Lcd.drawCircle(mapCenterX, mapCenterY, (mapRadius >> 1) + 1, WHITE);

    // draw lines at 0, 45, 90, 135 etc degrees azimuth
    for (int16_t i = 0; i <= 7; i++) {
      xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * mapRadius);
      yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * mapRadius);
      M5.Lcd.drawLine(mapCenterX, mapCenterY, xCoord + mapCenterX, yCoord + mapCenterY, WHITE);
      if(i % 2) continue;
      xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, 12, BLACK);
      char label;
      switch (i) {
        case 0: label = 'N'; break;
        case 2: label = 'E'; break;
        case 4: label = 'S'; break;
        case 6: label = 'W'; break;
      }
      M5.Lcd.drawChar(xCoord + mapCenterX - 5, yCoord + mapCenterY - 7, label, WHITE, BLACK, 2);
    }

    if (navsat->get_data().available) {

      // Set text size for satellite IDs
      M5.Lcd.setTextSize(1); // font is 6x8
      const auto& navsat_info = navsat->get_data().navsat_info;

      // draw the positions of the sats
      for(int16_t i = navsat_info.numSvsEphValid -1; i >= 0; i--) {

        // Sat position
        numSats ++;
        satAzimuth = navsat_info.svSortList[i].azim;
        satElevation = navsat_info.svSortList[i].elev;
        if(satElevation < 0) satElevation = 0;
        xCoord = round(-sin(radians(satAzimuth + 180 + compAngle)) *
                       map(satElevation, 0, 90, mapSatRadius, 1));
        yCoord = round(cos(radians(satAzimuth + 180 + compAngle)) *
                       map(satElevation, 0, 90, mapSatRadius, 1));

        // Sat ring color based on SNR
        if(navsat_info.svSortList[i].cno >= 35) {
          satRingColor = WHITE;
        } else if(navsat_info.svSortList[i].cno >=20) {
          satRingColor = YELLOW;
        } else {
          satRingColor = ORANGE;
        }
        M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, satRingRadius, satRingColor);

        // Sat color based on svUsed
        if(navsat_info.svSortList[i].svUsed) {
          satColor = BLUE;
        } else if(navsat_info.svSortList[i].healthy) {
          satColor = RED;
        } else {
          satColor = BLACK;
        }
        M5.Lcd.fillCircle(xCoord + mapCenterX, yCoord + mapCenterY, satRadius, satColor);

      }
    }
  }
}

void SatelliteViewScreen::enter_screen(Controller& controller) {
  controller.set_worker_active(k_worker_navsat_collector, true);
}

void SatelliteViewScreen::leave_screen(Controller& controller) {
  controller.set_worker_active(k_worker_navsat_collector, false);
}
