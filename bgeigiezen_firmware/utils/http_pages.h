#ifndef BGEIGIECAST_HTTP_PAGES_H
#define BGEIGIECAST_HTTP_PAGES_H

#include <WebServer.h>

#define TRANSMISSION_SIZE 4098

#define PURE_CSS_SIZE 3929
#define FAVICON_SIZE 696


class HttpPages {
 public:
  HttpPages() = delete;

  /**
   * Render the home page
   * @param device_id : To display the device id on the page
   * @return complete page rendered
   */
  static const char* get_home_page(
      uint32_t device_id
  );

  /**
   * Render the configuration page for device settings
   * @param display_success 
   * @param device_id 
   * @param led_intensity 
   * @param colorblind 
   * @return complete page rendered
   */
  static const char* get_config_device_page(
      bool display_success,
      uint32_t device_id
  );

  /**
   * Render the configuration page for connection settings
   * @param display_success 
   * @param device_id 
   * @param device_password 
   * @param wifi_ssid 
   * @param wifi_password 
   * @param api_key 
   * @param use_dev
   * @return complete page rendered
   */
  static const char* get_config_connection_page(
      bool display_success,
      uint32_t device_id
  );

  /**
   * Render the configuration page for location settings
   * @param display_success
   * @param device_id
   * @param use_home_location
   * @param home_latitude
   * @param home_longitude
   * @param last_latitude
   * @param last_longitude
   * @return complete page rendered
   */
  static const char* get_config_location_page(
      bool display_success,
      uint32_t device_id
  );

  static const uint8_t pure_css[PURE_CSS_SIZE];
  static const uint8_t favicon[FAVICON_SIZE];
  static const char* pure_js;

  static bool internet_access;

 private:

  static const char* render_full_page(uint32_t device_id, const char* page_name, const char* content_format, ...);
  static char transmission_buffer[TRANSMISSION_SIZE];

};

#endif //BGEIGIECAST_HTTP_PAGES_H
