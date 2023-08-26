
#include "api_connector.h"
#include "debugger.h"
#include "identifiers.h"

#define RETRY_TIMEOUT 10000
#define HOME_LOCATION_PRECISION_KM 0.4

// subtracting 1 seconds so data is sent more often than not.
#define SEND_FREQUENCY(last_send, sec, slack) (last_send == 0 || (millis() - last_send) > ((sec * 1000) - slack))

ApiConnector::ApiConnector(LocalStorage& config) :
    Handler(),
    _config(config),
    _last_success_send(0),
    _current_default_response(e_api_reporter_idle) {
}

bool ApiConnector::time_to_send(unsigned offset) const {
  switch (static_cast<SendFrequency>(_config.get_send_frequency())) {
    case e_api_send_frequency_5_sec:
      return SEND_FREQUENCY(_last_success_send, 5, offset);
    case e_api_send_frequency_1_min:
      return SEND_FREQUENCY(_last_success_send, 60, offset);
    case e_api_send_frequency_5_min:
    default:  // Default 5 min frequency
      return SEND_FREQUENCY(_last_success_send, 5 * 60, offset);
  }
}

bool ApiConnector::activate(bool retry) {
  static uint32_t last_try = 0;
  if (WiFiConnectionWrapper::wifi_connected()) {
    return true;
  }
  if (retry && millis() - last_try < RETRY_TIMEOUT) {
    return false;
  }
  last_try = millis();

  WiFiConnectionWrapper::connect_wifi(_config.get_wifi_ssid(), _config.get_wifi_password());

  return WiFi.isConnected();
}

void ApiConnector::deactivate() {
  _current_default_response = e_api_reporter_idle;
  WiFiConnectionWrapper::disconnect_wifi();
}

int8_t ApiConnector::handle_produced_work(const worker_map_t& workers) {
  const auto& data_worker = workers.worker<LogAggregator>(k_worker_log_aggregator);

  if (!data_worker || !data_worker->is_fresh()) {
    // No fresh data
    return _current_default_response;
  }

  if (!time_to_send()) {
    if (time_to_send(6000)) {
      // almost time to send, start wifi if not connected yet
      activate(true);
    }
    return _current_default_response;
  }

  const auto& line = data_worker->get_data();

  if (line.valid && line.in_fixed_range) {
    const unsigned long time_at_send = millis();
    _current_default_response = send_reading(line);

    if (_current_default_response == e_api_reporter_send_success) {
      _last_success_send = time_at_send;
    }
  }
  return _current_default_response;
}

ApiConnector::ApiHandlerStatus ApiConnector::send_reading(const DataLine& data) {

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
  DEBUG_PRINTF("POST complete, status %d\r\nrepsonse: \r\n\r\n%s\r\n\r\n", httpResponseCode, response.c_str());
  http.end();  //Free resources

  if (_config.get_send_frequency() != e_api_send_frequency_5_sec) {
    // Disconnect from wifi between readings (not needed when sending every 5 seconds)
    WiFiConnectionWrapper::disconnect_wifi();
  }

  switch (httpResponseCode) {
    case 200 ... 204:
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
      "\"longitude\":%.5f,"
      "\"latitude\":%.5f}\n",
      line.timestamp,
      _config.get_device_id(),
      line.cpm,
      _config.get_fixed_longitude(),
      _config.get_fixed_latitude()
  );
  return result > 0;
}

