#include <fsm_state_concrete.hpp>

StateStartup *StateStartup::_instance = NULL;
StateStartup *StateStartup::instance() {
  if (_instance == NULL) {
    _instance = new StateStartup();
  }
  return _instance;
}
void StateStartup::on_enter(bGeigieZen::Event e) {
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
void StateWaitGPSTime::on_enter(bGeigieZen::Event e) {}
void StateWaitGPSTime::process() {
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
void StateLogging::on_enter(bGeigieZen::Event e) {}
void StateLogging::process() {
  if (_ctx->geiger_count.available()) _ctx->on_geiger_counter_available();

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
