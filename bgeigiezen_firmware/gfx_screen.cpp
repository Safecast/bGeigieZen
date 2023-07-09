#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "gfx_screen.h"
#include "identifiers.h"
#include "controller.h"
#include "debugger.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "battery_indicator.h"

static constexpr uint8_t LEVEL_BRIGHT = 35;  // max brightness = 36
static constexpr uint8_t LEVEL_DIMMED = 10;
static constexpr uint8_t LEVEL_BLANKED = 0;
static constexpr uint32_t DELAY_DIMMING_DEFAULT = 2 * 60 * 1000;  // ms before dimming screen
static constexpr uint32_t DELAY_BLANKING_DEFAULT = 3 * 60 * 1000;  // ms before blanking screen


GFXScreen::GFXScreen() : Supervisor(), _last_render(0) {
}

void GFXScreen::initialize() {
  // Initialize TFT (may already be done by M5)
  off();
}

void GFXScreen::off() {
  // Clear the graphics screen
  M5.Lcd.clear();
  setBrightness(LEVEL_BLANKED);
}

/**
 * @brief Draw the boot-up welcome screen
 * 
 */
void GFXScreen::screenSplash() {
  /// @todo drawsplash should display the device name, ID, F/W version and copyright
//  clear();
  M5.Lcd.setRotation(3);
  // Display something
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawString("bGeigie Zen", 90, 50, 4);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Some splash screen device details.", 5, 100, 2);
  M5.Lcd.drawString("device name, ID, version", 5, 120, 2);
  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);
  M5.Lcd.setRotation(1);
}

/**
 * @brief Write SD error text
 * 
 */
void GFXScreen::screenSDError() {
//  clear();
  M5.Lcd.setRotation(3);
  // Display SD error
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawString("No SDCARD in slot", 50, 50, 4);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Insert a SDCARD and press any button to restart.", 5, 90, 2);
  //display Safecast copyright
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);
  M5.Lcd.setRotation(1);
}

/**
 * @brief Draw main dashboard
 *
 */
void GFXScreen::screenDashboard(const worker_map_t& workers, const handler_map_t& handlers) {
  ///
//  clear();
  M5.Lcd.setRotation(3);
  // Display something
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawRoundRect(20, -5, 90, 20, 4, TFT_WHITE);
  M5.Lcd.drawString("Button C", 40, 8);
  M5.Lcd.drawRoundRect(115, -5, 90, 20, 4, TFT_WHITE);
  M5.Lcd.drawString("Button B", 135, 8);
  M5.Lcd.drawRoundRect(210, -5, 90, 20, 4, TFT_WHITE);
  M5.Lcd.drawString("Button A", 230, 8);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setCursor(0, 30);
  const auto& gm_sensor = workers.worker<GeigerMullerSensor>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);
  M5.Lcd.printf("CPM: %d\n",
                gm_sensor->get_data().CPM);
  M5.Lcd.printf("GPS status: %s %d\n",
                gps->get_data().available ? "Available" : "Unavailable",
                gps->get_data().satellites);
  M5.Lcd.printf("GPS lat,long: %.5f,%.5f\n",
                gps->get_data().latitude,
                gps->get_data().longitude);
  M5.Lcd.printf("Battery: %d%% %s\n",
                battery->get_data().percentage,
                battery->get_data().isCharging ? "(charging)" : "");
  M5.Lcd.setRotation(1);
}

//setup brightness by Rob Oudendijk 2023-03-13
void GFXScreen::setBrightness(uint8_t lvl, bool overdrive) {
#ifdef M5_CORE2
  // The backlight brightness is in steps of 25 in AXP192.cpp
  // calculation in SetDCVoltage: ( (voltage - 700) / 25 )
  // 2325 is the minimum "I can just about see a glow in a dark room" level of brightness.
  // 2800 is the value set by the AXP library as "standard" bright backlight.
  int v = lvl * 25 + 2300;

  // Clamp to safe values.
  if (v < 2300) v = 2300;
  if (overdrive) {
    if (v > 3200) v = 3200; // maximum of 3.2 volts, 3200 (uint8_t lvl  = 36) absolute max!
  } else {
    if (v > 2800) v = 2800; // maximum of 2.8 volts, 2800 (uint8_t lvl  = 20)
  }

  // Minimum brightness means turn off the LCD backlight.
  if (v == 2300) {
    // LED set to minimum brightness? Turn off.

    M5.Axp.SetDCDC3(false);
    return;
  } else {
    // Ensure backlight is on. (magic name = DCDC3)
    M5.Axp.SetDCDC3(true);
  }

  // Set the LCD backlight voltage. (magic number = 2)

  M5.Axp.SetDCVoltage(2, v);

#elif M5_BASIC
  // M5Stack Basic uses LCD Brightness (0: Off - 255:Full)
  int v = lvl * 25;
  M5.Lcd.setBrightness(v);
#endif
}

void GFXScreen::clear() {
  // Clear display
  M5.Lcd.clear();
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  setBrightness(LEVEL_BRIGHT);
}

void GFXScreen::handle_report(const worker_map_t& workers, const handler_map_t& handlers) {
  const auto& control = workers.worker<Controller>(k_worker_controller_state);
  if (!control->is_fresh() && _last_render + 1000 > millis()) {
    return;
  }
  if (control->is_fresh()) {
    clear();
  }
  _last_render = millis();

  switch (control->get_data()) {
    case k_state_InitializeDeviceState:
      break;
    case k_state_EmptySDCardState:
      screenSDError();
      break;
    case k_state_NoSDCardState:
      screenSDError();
      break;
    case k_state_PostInitializeState:
      screenSplash();
      break;
    case k_state_ConfigurationModeState:
      break;
    case k_state_MeasurementModeState:
      screenDashboard(workers, handlers);
      break;
    case k_state_ResetState:
      break;
  }
}
