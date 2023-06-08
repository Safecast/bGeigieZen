#ifndef BGEIGIEZEN_SERVER_H
#define BGEIGIEZEN_SERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <display.hpp>

// #include "local_storage.h" /** TODO: Figure out where to write the configuration */
#include <wifi_connection.hpp>

enum ServerStatus {
  k_server_status_offline,
  k_server_status_running_wifi,
  k_server_status_running_access_point,
};

/**
 * Class to host a web server for configuring the ESP32. Will set up an access
 * point based on config.hpp "Access point settings".
 */
class ConfigWebServer {
 public:
  explicit ConfigWebServer(Display& display);
  virtual ~ConfigWebServer() = default;

  /**
   * Checks if there are requests and handles them
   */
  int8_t produce_data();

  /**
   * Initialize the web server and endpoints
   */
  void add_urls();

  //void handle_report(const Report& report) override;

 protected:
  /**
   * Initialize the web server, does nothing if it is already initialized.
   * @return true if initialization was success
   */
  bool activate(bool retry) override;

 protected:
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
  // LocalStorage& _config;
};

#endif //BGEIGIECAST_SERVER_H
