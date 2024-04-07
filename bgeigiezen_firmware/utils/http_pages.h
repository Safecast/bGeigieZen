#ifndef BGEIGIECAST_HTTP_PAGES_H
#define BGEIGIECAST_HTTP_PAGES_H

#include <WebServer.h>
#include <workers/local_storage.h>

#define TRANSMISSION_SIZE 6000

#define PURE_CSS_SIZE 3929
#define FAVICON_SIZE 696



// Device setting form names
#define FORM_NAME_ALERT_THRESHOLD "d_al"
#define FORM_NAME_CPM_USVH "d_cu"
#define FORM_NAME_MANUAL_LOGGING "d_ml"
#define FORM_NAME_ENABLE_JOURNAL "d_ej"
#define FORM_NAME_LOG_VOID "d_lv"
#define FORM_NAME_SCREEN_DIM_TIMEOUT "d_dt"
#define FORM_NAME_SCREEN_OFF_TIMEOUT "d_ot"
#define FORM_NAME_ANIMATED_SCREENSAVER "d_as"

// Connection setting form names
#define FORM_NAME_AP_LOGIN "c_ap"
#define FORM_NAME_WIFI_SSID "c_ws"
#define FORM_NAME_WIFI_PASS "c_wp"
#define FORM_NAME_API_KEY "c_ak"

// Location setting form names
#define FORM_NAME_LOC_FIXED_LAT "l_ha"
#define FORM_NAME_LOC_FIXED_LON "l_ho"


class HttpPages {
 public:
  HttpPages() = delete;

  /**
   * Render the home page
   * @param device_id : To display the device id on the page
   * @return complete page rendered
   */
  static const char* get_home_page(
      const LocalStorage& settings
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
      const LocalStorage& settings
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
      const LocalStorage& settings
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
      const LocalStorage& settings
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
