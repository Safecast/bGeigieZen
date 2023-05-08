#include <fsm_state_concrete.hpp>

StateStartup *StateStartup::_instance = NULL;
StateStartup *StateStartup::instance() {
  if (_instance == NULL) {
    _instance = new StateStartup();
  }
  return _instance;
}
void StateStartup::on_enter(bGeigieZen::Event e) {
  Serial.println("StateStartup::on_enter");
  _ctx->setup();
  _ctx->transition_to(StateWaitGPSTime::instance(), bGeigieZen::Event::SETUP_FINISHED);
}
void StateStartup::process() {}
void StateStartup::on_exit(bGeigieZen::Event e) {}

StateWaitGPSTime *StateWaitGPSTime::_instance = NULL;
StateWaitGPSTime *StateWaitGPSTime::instance() {
  if (_instance == NULL) {
    _instance = new StateWaitGPSTime();
  }
  return _instance;
}
void StateWaitGPSTime::on_enter(bGeigieZen::Event e) {
  Serial.println("StateWaitGPSTime::on_enter");
  auto menuinst = StateMenu::instance();
  auto inst = instance();
  auto ctx = inst->_ctx;
  Serial.println("StateWaitGPSTime::on_enter Adding handler");
  M5.BtnA.addHandler([ctx, menuinst](Event &e) {ctx->transition_to(menuinst, bGeigieZen::Event::MENU_INVOKED);}, E_RELEASE);
}
void StateWaitGPSTime::process() {
  Serial.printf("StateWaitGPSTime::process updating the display\n");
  _ctx->display.update();  // redraws the display
  if (_ctx->geiger_count.available()) _ctx->on_geiger_counter_available();

  if (_ctx->gps.available()) {
    _ctx->on_gps_available();

    if (_ctx->gps.time_valid())
      _ctx->transition_to(StateLogging::instance(), bGeigieZen::Event::GPS_DATE_BECAME_CORRECT);
  }
}
void StateWaitGPSTime::on_exit(bGeigieZen::Event e) {
  if (e == bGeigieZen::Event::GPS_DATE_BECAME_CORRECT) {
    TinyGPSDate &date = _ctx->gps.data().date;
    uint32_t dev_id = _ctx->device_setup.config().device_id;
    bool ret = _ctx->logger.start(dev_id, date.year(), date.month(), date.day());
    if (!ret)
      Serial.println("Log creation failed");
    else
      Serial.println("Log creation succeeded");
  }
}

StateLogging *StateLogging::instance() {
  if (_instance == NULL) {
    _instance = new StateLogging();
  }
  return _instance;
}
StateLogging *StateLogging::_instance = NULL;
void StateLogging::on_enter(bGeigieZen::Event e) {
  Serial.println("StateLogging::on_enter");
}
void StateLogging::process() {
  if (_ctx->geiger_count.available()) _ctx->on_geiger_counter_available();
// auto wifi = _ctx->device_setup.wifiparams();
// wifi.configIPaddr
  if (_ctx->gps.available()) _ctx->on_gps_available();

  if (_ctx->bgeigie_formatter.ready()) {
    Serial.println(_ctx->bgeigie_formatter.format());
    if (!_ctx->sd_wrapper.ready())
      Serial.println("SD card could not be started");
    else if (!_ctx->logger.folder_created()) {
      Serial.println("Folder creation failed.");
      _ctx->logger.start(
          _ctx->device_setup.config().device_id, _ctx->gps.data().date.year(),
          _ctx->gps.data().date.month(), _ctx->gps.data().date.day());
    } else if (!_ctx->logger.ready())
      Serial.println("Log creation failed");
    else {
      bool ret = _ctx->logger.log(_ctx->bgeigie_formatter.format());

      if (!ret) Serial.println("Log to sd card failed");
    }
  }
}
void StateLogging::on_exit(bGeigieZen::Event e) {}

StateConfig *StateConfig::instance() {
  if (_instance == NULL) {
    _instance = new StateConfig();
  }
  return _instance;
}
StateConfig *StateConfig::_instance = NULL;
void StateConfig::on_enter(bGeigieZen::Event e) {
  Serial.println("StateConfig::on_enter");
  if(e == bGeigieZen::Event::CONFIGURATION_INVOKED) {
    // Launch the configuration web server here
  }
}
void StateConfig::process() {
}
void StateConfig::on_exit(bGeigieZen::Event e) {}


StateMenu *StateMenu::instance() {
  if (_instance == NULL) {
    _instance = new StateMenu();
  }
  return _instance;
}
StateMenu *StateMenu::_instance = NULL;
void StateMenu::on_enter(bGeigieZen::Event e) {
  Serial.println("StateMenu::on_enter");
  if(e == bGeigieZen::Event::MENU_INVOKED) {
    // De-register the BtnA callback
    M5.BtnA.delHandlers();
    // Draw the menu display
  }
}
void StateMenu::process() {
}
void StateMenu::on_exit(bGeigieZen::Event e) {}

/* // Button callback to force a state change
void State::register_callback(std::function<void(Event &)> callback, uint16_t event_type){
  // Activate hook on button 
  Serial.println("MENU BUTTON CALLBACK REGISTERED");
  
  M5.BtnA.addHandler(callback, event_type);
}

EventHandlerCallback StateWaitGPSTime::invoke_menu(Event& e) { 
  Serial.printf("StateWaitGPSTime::invoke_menu CALLBACK EVENT: %04x\n", e.type);
  _ctx->transition_to(StateMenu::instance(), bGeigieZen::Event::MENU_INVOKED);
}

EventHandlerCallback StateLogging::invoke_menu(Event& e) { 
  Serial.printf("StateLogging::invoke_menu CALLBACK EVENT: %04x\n", e.type);
  _ctx->transition_to(StateMenu::instance(), bGeigieZen::Event::MENU_INVOKED);
}

 */