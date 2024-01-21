
#include "api_connector.h"
#include "debugger.h"
#include "identifiers.h"
#include "workers/zen_button.h"

#define RETRY_TIMEOUT 10000

// subtracting 1 seconds so data is sent more often than not.
#define SEND_FREQUENCY(last_send, sec) (last_send == 0 || (millis() - last_send) > ((sec * 1000) - 500))

ApiConnector::ApiConnector(LocalStorage& config) : Handler(), _config(config), _post_count(0), _last_post(0), _testing_mode(false) {
}

bool ApiConnector::time_to_send(bool alert) const {
  if (_testing_mode) {
    return SEND_FREQUENCY(_last_post, API_SEND_SECONDS_DELAY_TESTING);
  }
  return SEND_FREQUENCY(_last_post, alert ? API_SEND_SECONDS_DELAY_ALERT : API_SEND_SECONDS_DELAY);
}

bool ApiConnector::activate(bool retry) {
  static uint32_t last_try = 0;
  if (WiFiWrapper_i.wifi_connected()) {
    return true;
  }
  if (retry && millis() - last_try < RETRY_TIMEOUT) {
    return false;
  }
  last_try = millis();

  WiFiWrapper_i.connect_wifi(_config.get_wifi_ssid(), _config.get_wifi_password(), !retry);

  return WiFi.isConnected();
}

void ApiConnector::deactivate() {
  WiFiWrapper_i.disconnect_wifi();
}

int8_t ApiConnector::handle_produced_work(const worker_map_t& workers) {
  auto testing_mode = workers.worker<ZenButton>(k_worker_button_1);
  if (testing_mode->is_fresh()) {
    _testing_mode = testing_mode->get_data().longPress;
    return e_api_reporter_idle; // Return for now to apply changes first
  }

  if (!_config.get_fixed_device_id()) {
    // Cant even send anything, because there is an invalid id
    return e_api_reporter_idle;
  }

  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  const auto& log_data = log_aggregator->get_data();

  if (!log_aggregator->is_fresh()) {
    // No fresh data
    return e_api_reporter_idle;
  }

  if (!log_data.valid()) {
    // data not valid (either gm or gps)
    return e_api_reporter_idle;
  }

  if (!time_to_send(log_data.cpm > _config.get_alarm_threshold())) {
    // Not time to send yet
    return e_api_reporter_idle;
  }
  
  if (_testing_mode || log_data.in_fixed_range) {
    return send_reading(log_data);
  }

  // Not in fixed range or not testing, so not sending
  return e_api_reporter_idle;
}

ApiConnector::ApiHandlerStatus ApiConnector::send_reading(const DataLine& data) {
  const unsigned long time_at_send = millis();

  if (!WiFi.isConnected() && !activate(true)) {
    DEBUG_PRINTLN("Unable to send, lost connection");
    return e_api_reporter_error_not_connected;
  }

  HTTPClient http;

  char url[100];
  sprintf(url, "%s?api_key=%s&", API_MEASUREMENTS_ENDPOINT, _config.get_api_key());

  //Specify destination for HTTP request
  if (!http.begin(url)) {
    DEBUG_PRINTLN("Unable to begin url connection");
    http.end();  //Free resources
    return e_api_reporter_error_remote_not_available;
  }

  char payload[200];

  if (!reading_to_json(data, payload)) {
    return e_api_reporter_error_to_json;
  }

  char content_length[5];

  sprintf(content_length, "%d", strlen(payload));

  http.setUserAgent(HEADER_API_USER_AGENT);
  http.addHeader("Host", API_HOST);
  http.addHeader("Content-Type", HEADER_API_CONTENT_TYPE);
  http.addHeader("Content-Length", content_length);

  int httpResponseCode = http.POST(payload);

  String response = http.getString();
  DEBUG_PRINTF("POST complete, response (%d):\n%s\n\n", httpResponseCode, response.c_str());
  http.end();  //Free resources

  // TODO: Check response measurement ID

  WiFiWrapper_i.update_active();
  _last_post = time_at_send;

  switch (httpResponseCode) {
    case 200 ... 204:
      ++_post_count;
      return e_api_reporter_send_success;
    case 400:
      return e_api_reporter_error_server_rejected_post_400;
    case 401:
      return e_api_reporter_error_server_rejected_post_401;
    case 403:
      return e_api_reporter_error_server_rejected_post_403;
    case 500 ... 599:
      return e_api_reporter_error_server_rejected_post_5xx;
    default:
      return e_api_reporter_error_remote_not_available;
  }

}

bool ApiConnector::reading_to_json(const DataLine& line, char* out) {
  auto result = sprintf(
      out,
      "{\"captured_at\":\"%s\","
      "\"device_id\":%d,"
      "\"value\":%d,"
      "\"unit\":\"cpm\","
      "\"latitude\":%0.6f,"
      "\"longitude\":%0.6f}\n",
      line.timestamp,
      _config.get_fixed_device_id(),
      line.cpm,
      _testing_mode ? line.latitude : _config.get_fixed_latitude(),
      _testing_mode ? line.longitude : _config.get_fixed_longitude()
  );
  return result > 0;
}

bool ApiConnector::testing_mode() const {
  return _testing_mode;
}

uint32_t ApiConnector::get_post_count() const {
  return _post_count;
}
