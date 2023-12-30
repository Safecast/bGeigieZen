#ifndef BGEIGIEZEN_BASE_SCREEN_H_
#define BGEIGIEZEN_BASE_SCREEN_H_

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "controller.h"
#include <Supervisor.hpp>

#define STATUS_ERROR_GEIGER F(" NO GEIGER TUBE CONNECTED ");
#define STATUS_ERROR_GPS F(" NO GPS MODULE CONNECTED ");
#define STATUS_ERROR_SD F(" NO SD CARD INSERTED ");
#define STATUS_ERROR_WIFI_NO_SSID_AVAIL F(" CONFIGURED SSID NOT AVAILABLE ");
#define STATUS_ERROR_WIFI_CONNECT_FAILED F(" WIFI CONNECTION FAILED ");
#define STATUS_ERROR_WIFI_CONNECTION_LOST F(" WIFI CONNECTION LOST ");



class BaseScreen {
 public:
  enum ButtonState {
    e_button_default,
    e_button_active,
    e_button_disabled
  };

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) = 0;
  /**
   * Enter the screen, use controller to enable screen specific workers/handlers
   * @param controller
   */
  virtual void enter_screen(Controller& controller);

  /**
   * Leave the screen, use controller to disable screen specific workers/handlers
   * @param controller
   */
  virtual void leave_screen(Controller& controller);

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void do_render(const worker_map_t& workers, const handler_map_t& handlers) final;

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void force_next_render() final;

  /**
   * Get screen name
   * @return
   */
  virtual const char* get_title() const;

  /**
   * Get screen name
   * @return
   */
  virtual bool has_status_bar() const;

  /**
   * Get screen error message (if available), else nullptr
   * @return
   */
  virtual const __FlashStringHelper* get_error_message(const worker_map_t& workers, const handler_map_t& handlers) const;

  /**
   * Get screen status message (if available), else nullptr
   * @return
   */
  virtual const __FlashStringHelper* get_status_message(const worker_map_t& workers, const handler_map_t& handlers) const;

  bool has_required_gps() const { return required_gps; }
  bool has_required_sd() const { return required_sd; }
  bool has_required_tube() const { return required_tube; }
  bool has_required_wifi() const { return required_wifi; }
  bool has_required_ble() const { return required_ble; }

 protected:
  explicit BaseScreen(const char* title, bool status_bar);
  virtual ~BaseScreen() = default;

  /**
   * Render the screen with latest data
   * @param workers
   * @param handlers
   */
  virtual void render(const worker_map_t& workers, const handler_map_t& handlers, bool force);

  void drawButton1(const char* text, ButtonState state = e_button_default) const;
  void drawButton2(const char* text, ButtonState state = e_button_default) const;
  void drawButton3(const char* text, ButtonState state = e_button_default) const;

  int printFloatFont(float val, int prec, int x, int y, int font) const;
  int printIntFont(unsigned long val, int x, int y, int font) const;

  void set_status_message(const __FlashStringHelper* message);

  // required modules for status bar
  bool required_gps;
  bool required_sd;
  bool required_tube;
  bool required_wifi;
  bool required_ble;

 private:
  void drawButton(uint16_t x, const char* text, ButtonState state) const;

  char _title[20];
  bool _status_bar;
  bool _force_render;
  uint32_t _status_message_time;
  const __FlashStringHelper* _message;
};

#endif //BGEIGIEZEN_BASE_SCREEN_H_
