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

    if (gps->active() && !navsat->active()) {
      // reconnect navsat worker once gps worker is connected
      // M5_LOGD("NAVSAT: Activating navsat collector");
      controller.set_worker_active(k_worker_navsat_collector, true);
      if (navsat->active()) {
        set_status_message(F(" Nav-sat connected, this can take a few seconds "));
        // M5_LOGD("NAVSAT: Successfully activated");
      } else {
        set_status_message(F(" Nav-sat was unable to connect "));
        // M5_LOGD("NAVSAT: Failed to activate");
      }
    }

    const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    const auto button3 = workers.worker<ZenButton>(k_worker_button_3);
    if (button1->is_fresh() && button1->get_data().shortPress) {
      open_menu(true);
      // Don't clear the entire screen to avoid flickering
      // The menu render will handle clearing what it needs
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

  // M5_LOGD("SAT: active=%d fresh=%d avail=%d numSvs=%d", navsat->active(), navsat->is_fresh(), 
  //        navsat->get_data().available, navsat->get_data().available ? navsat->get_data().navsat_info.numSvsEphValid : 0);

  if (force || (navsat && navsat->is_fresh())) {

    constexpr int16_t mapRadius   = 90;
    constexpr int16_t mapSatRadius = 80; // Keep sat graphics inside map
    constexpr int16_t mapCenterX  = 108;
    constexpr int16_t mapCenterY  = 110;
    constexpr int16_t mapSize     = mapRadius * 2;
    constexpr int16_t compAngle   = 0;
    constexpr int16_t satRadius   = 10;
    constexpr int16_t satRingRadius = 12;

    // Render the constellation map into a sprite and push it atomically to
    // avoid the clear→draw flicker that happens when writing directly to LCD.
    static M5Canvas mapSprite(&M5.Lcd);
    if (!mapSprite.width()) {
      mapSprite.createSprite(mapSize, mapSize);
    }
    mapSprite.fillSprite(LCD_COLOR_BACKGROUND);

    // Draw grid: two range circles
    mapSprite.drawCircle(mapRadius, mapRadius, mapRadius, LCD_COLOR_DEFAULT);
    mapSprite.drawCircle(mapRadius, mapRadius, (mapRadius >> 1) + 1, LCD_COLOR_DEFAULT);

    // Draw compass lines and N/E/S/W labels
    for (int16_t i = 0; i <= 7; i++) {
      int16_t xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * mapRadius);
      int16_t yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * mapRadius);
      mapSprite.drawLine(mapRadius, mapRadius, xCoord + mapRadius, yCoord + mapRadius, LCD_COLOR_DEFAULT);
      if (i % 2) continue;
      xCoord = round(-sin(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      yCoord = round(cos(radians((i * 45) + 180 + compAngle)) * (mapRadius - 8));
      mapSprite.fillCircle(xCoord + mapRadius, yCoord + mapRadius, 12, LCD_COLOR_BACKGROUND);
      char label;
      switch (i) {
        case 0: label = 'N'; break;
        case 2: label = 'E'; break;
        case 4: label = 'S'; break;
        default: label = 'W'; break;
      }
      mapSprite.drawChar(xCoord + mapRadius - 5, yCoord + mapRadius - 7, label, LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT, 2);
    }

    if (navsat->get_data().available) {
      const auto& navsat_info = navsat->get_data().navsat_info;
      char _dispStr[4];

      for (int16_t i = navsat_info.numSvsEphValid - 1; i >= 0; --i) {
        int16_t satAzimuth   = navsat_info.svSortList[i].azim;
        int8_t  satElevation = navsat_info.svSortList[i].elev;
        if (satElevation < 0) satElevation = 0;
        int16_t xCoord = round(-sin(radians(satAzimuth + 180 + compAngle)) *
                               map(satElevation, 0, 90, mapSatRadius, 1));
        int16_t yCoord = round(cos(radians(satAzimuth + 180 + compAngle)) *
                               map(satElevation, 0, 90, mapSatRadius, 1));

        int32_t satRingColor;
        if (navsat_info.svSortList[i].cno >= 35)      satRingColor = COLOR_SAT_SIGNAL_STRONG;
        else if (navsat_info.svSortList[i].cno >= 20) satRingColor = COLOR_SAT_SIGNAL_MEDIUM;
        else                                           satRingColor = COLOR_SAT_SIGNAL_WEAK;
        mapSprite.fillCircle(xCoord + mapRadius, yCoord + mapRadius, satRingRadius, satRingColor);

        int32_t satColor;
        if (navsat_info.svSortList[i].svUsed)        satColor = COLOR_SAT_USED_NAV;
        else if (navsat_info.svSortList[i].healthy)  satColor = COLOR_SAT_HEALTHY;
        else                                          satColor = COLOR_SAT_UNKNOWN;
        mapSprite.fillCircle(xCoord + mapRadius, yCoord + mapRadius, satRadius, satColor);

        mapSprite.setTextColor(LCD_COLOR_DEFAULT, satColor);
        sprintf(_dispStr, "%c%02d",
                navsat_info.svSortList[i].gnssIdType,
                navsat_info.svSortList[i].svId);
        mapSprite.setTextDatum(5); // middle_center
        mapSprite.drawString(_dispStr, xCoord + mapRadius, yCoord + mapRadius, &fonts::Font0);
        mapSprite.setTextDatum(0); // reset to top_left
      }
    }

    // Push the complete map to the LCD in one operation — no visible clear flash
    mapSprite.pushSprite(mapCenterX - mapRadius, mapCenterY - mapRadius);
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
