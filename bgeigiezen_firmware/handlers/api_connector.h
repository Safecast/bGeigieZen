#ifndef BGEIGIEZEN_APICONNECTOR_H_
#define BGEIGIEZEN_APICONNECTOR_H_

#include <WiFi.h>
#include <HTTPClient.h>

#include <Handler.hpp>

#include "workers/local_storage.h"
#include "user_config.h"
#include "utils/wifi_connection.h"
#include "workers/log_aggregator.h"

/**
 * Connects over WiFi to the API to send readings
 */
class ApiConnector : public Handler {
 public:
  /**
   * Collection of statuses for the api data handling, default is idle.
   */
  enum ApiHandlerStatus {
    e_api_reporter_idle,
    e_api_reporter_send_success,
    e_api_reporter_error_to_json,
    e_api_reporter_error_not_connected,
    e_api_reporter_error_remote_not_available,
    e_api_reporter_error_server_rejected_post_400,
    e_api_reporter_error_server_rejected_post_401,
    e_api_reporter_error_server_rejected_post_403,
    e_api_reporter_error_server_rejected_post_5xx,
  };

  explicit ApiConnector(LocalStorage& config);
  virtual ~ApiConnector() = default;

  bool testing_mode() const;
  uint32_t get_post_count() const;

 protected:

  /**
   * Check if enough time has passed to send the latest reading to api
   * @param offset: additional ms offset (default 1 sec to overlap measurements better)
   * @return true if time to send
   */
  bool time_to_send(bool alert) const;

  /**
   * Initialize the connection
   * @param initial: set to false if its for reconnect / connect in error
   * @return true if connection with the wifi was made
   */
  bool activate(bool retry) override;

  /**
   * Stop the connection
   */
  void deactivate() override;

  int8_t handle_produced_work(const worker_map_t& workers) override;

  bool reading_to_json(const DataLine& reading, char* out);

 private:

  /**
   * Send a reading to the API
   * @param reading: reading to send
   * @return: true if the API call was successful
   */
  ApiHandlerStatus send_reading(const DataLine& reading);

  LocalStorage& _config;
  uint32_t _post_count;
  uint32_t _last_post;
  bool _testing_mode;
};

#endif //BGEIGIEZEN_APICONNECTOR_H_
