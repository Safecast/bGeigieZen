
#include <M5Core2.h> // #include <M5Stack.h>

#include <fsm_context.hpp>

void Context::begin() {
  transition_to(StateStartup::instance(), bGeigieZen::Event::DEVICE_STARTED);
}

void Context::loop() {
  M5.update();  // updates buttons and stuff
  geiger_count.update();
  gps.update();      // reads from the GPS
  display.update();  // redraws the display and manages the states

  current_state->process();
}

void Context::setup() {
  // we put a delay at the beginning so that we can open the serial for debug
  delay(100);

  // Serial setup for M5Stack
  M5.begin();
  Serial.begin(115200);

  // Start I2C communications for battery level indicator
  Wire.begin();

  // start the SD card for the logger and the configuration file
  auto ret = sd_wrapper.begin();
  while (!ret) {

    Serial.println("Error:");
    Serial.println("No SDCARD in slot");
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.lcd.setRotation(3);
    M5.Lcd.drawString("No SDCARD in slot", 5, 50, 4);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.drawString("Insert a SDCARD and restart.", 5, 90, 2);
    //display Safecast copyright
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.drawString("SAFECAST", 230, 215, 1);
    M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Lcd.drawString("2021", 285, 215, 1);
    delay(2000);
  }

  // Once we have initialized the SD card, we can read the configuration of the
  // device
  device_setup.initialize();
  if (sd_wrapper.ready()) {
    bool success = device_setup.load_from_file(SETUP_FILENAME);
    if (!success) Serial.println("Failed to read config from file");
  }

  // Now we can setup the device ID in the logger
  geiger_count.configure(device_setup.config().cpm2ush_divider,
                         device_setup.config().cpm2bqm2_factor,
                         device_setup.config().alarm_level);
  bgeigie_formatter.set_device_id(device_setup.config().device_id);
  display.feed(device_setup);

  // Reset the pulse counter before starting
  geiger_count.begin();
}

void Context::on_geiger_counter_available() {
  // mark the Geiger data as not fresh
  geiger_count.consume();

  // immediately update the data consummers
  bgeigie_formatter.feed(geiger_count);
  display.feed(geiger_count);
  display.feed_battery_level(battery_monitor.get_level());
}

void Context::on_gps_available() {
  display.feed(gps);
  bgeigie_formatter.feed(gps);
}

void Context::transition_to(State *next_state, bGeigieZen::Event e) {
  if (current_state != NULL) current_state->exit(e);
  current_state = next_state;
  current_state->enter(this, e);
}
