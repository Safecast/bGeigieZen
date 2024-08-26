#include "api_connector.h"
#include "identifiers.h"

#define RETRY_TIMEOUT 10000

// subtracting 1 seconds so data is sent more often than not.
#define SEND_FREQUENCY(last_send, sec) ((last_send) == 0 || (millis() - (last_send)) > (((sec) * 1000) - 500))

ApiConnector::ApiConnector(LocalStorage& config) : Handler(), _http_client(), _config(config), _payload(""), _post_count(0), _last_post(0) {
}

bool ApiConnector::time_to_send(bool in_fixed_range, bool alert) const {
  if (in_fixed_range) {
    return SEND_FREQUENCY(_last_post, alert ? API_SEND_SECONDS_DELAY_ALERT : API_SEND_SECONDS_DELAY);
  }
  return SEND_FREQUENCY(_last_post, API_SEND_SECONDS_DELAY_ROAMING);
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
  kill_task();
  WiFiWrapper_i.disconnect_wifi();
}

int8_t ApiConnector::handle_produced_work(const worker_map_t& workers) {
  if (!_config.get_fixed_device_id()) {
    // Cant even send anything, because there is an invalid id
    return e_handler_idle;
  }

  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  const auto& log_data = log_aggregator->get_data();

  if (!log_aggregator->is_fresh()) {
    // No fresh data
    return e_handler_idle;
  }

  if (!log_data.valid()) {
    // data not valid (either gm or gps)
    return e_handler_idle;
  }

  if (!time_to_send(log_data.in_fixed_range, log_data.cpm > _config.get_alert_threshold())) {
    // Not time to send yet
    return e_handler_idle;
  }

  if (!WiFi.isConnected() && !activate(true)) {
    M5_LOGD("Unable to send, lost connection");
    return e_api_reporter_error_not_connected;
  }

  if (!reading_to_json(log_data,_payload)) {
    return e_api_reporter_error_to_json;
  }

  M5_LOGD("Starting task to send data to API...");
  return start_task("api_send", 2048 * 4, 3);
}

bool ApiConnector::reading_to_json(const DataLine& line, char* out) {
  auto result = sprintf(
      out,
      "{\"captured_at\":\"%s\","
      "\"device_id\":%d,"
      "\"value\":%d,"
      "\"unit\":\"cpm\","
      "\"height\":%0.6f,"
      "\"latitude\":%0.6f,"
      "\"longitude\":%0.6f}",
      line.timestamp,
      _config.get_fixed_device_id(),
      line.cpm,
      line.in_fixed_range ? 0 : line.latitude,  // TODO: fixed location altitude
      line.in_fixed_range ? _config.get_fixed_latitude() : line.latitude,
      line.in_fixed_range ? _config.get_fixed_longitude() : line.longitude);
  return result > 0;
}

uint32_t ApiConnector::get_post_count() const {
  return _post_count;
}

int8_t ApiConnector::handle_async() {
  const unsigned long time_at_send = millis();

  char url[100];
  sprintf(url, "%s?api_key=%s&", API_MEASUREMENTS_ENDPOINT, _config.get_api_key());

  //Specify destination for HTTP request
  if (!_http_client.begin(url)) {
    M5_LOGD("Unable to begin url connection");
    _http_client.end(); //Free resources
    return e_api_reporter_error_remote_not_available;
  }

  char content_length[5];

  sprintf(content_length, "%d", strlen(_payload));

  _http_client.setUserAgent(HEADER_API_USER_AGENT);
  _http_client.addHeader("Host", API_HOST);
  _http_client.addHeader("Content-Type", HEADER_API_CONTENT_TYPE);
  _http_client.addHeader("Content-Length", content_length);

  int httpResponseCode = _http_client.POST(_payload);

  String response = _http_client.getString();
  M5_LOGD("POST complete, response (%d):\n%s\n", httpResponseCode, response.c_str());
  _http_client.end(); //Free resources

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