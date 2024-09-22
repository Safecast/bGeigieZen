#ifndef BGEIGIEZEN_SERVER_H_
#define BGEIGIEZEN_SERVER_H_

#include <WiFi.h>
#include <WebServer.h>

#include <Worker.hpp>
#include <Supervisor.hpp>

#include "local_storage.h"
#include "utils/wifi_connection.h"

enum ServerStatus {
  k_server_status_offline,
  k_server_status_running_wifi,
  k_server_status_running_access_point,
};

/**
 * Class to host a web server for configuring the ESP32. Will set up an access
 * point based on user_config.h "Access point settings".
 */
class ConfigWebServer : public Worker<ServerStatus> {
 public:
  explicit ConfigWebServer(LocalStorage& config);
  virtual ~ConfigWebServer() = default;

  /**
   * Checks if there are requests and handles them
   */
  int8_t produce_data() override;

  /**
   * Initialize the web server and endpoints
   */
  void add_urls();

 protected:
  /**
   * Initialize the web server, does nothing if it is already initialized.
   * @return true if initialization was success
   */
  bool activate(bool retry) override;

  /**
   * Stops the web server
   */
  void deactivate() override;

 private:

  /**
   * Handles request for `/save`
   */
  void handle_save();

  /**
   * Handles request for `/update` post
   */
  void handle_update_uploading();

  WebServer _server;
  LocalStorage& _config;
  bool _handled_client;
};

#endif //BGEIGIEZEN_SERVER_H_
