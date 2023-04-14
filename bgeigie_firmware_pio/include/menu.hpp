#ifndef __MENU_H__
#define __MENU_H__

#include <M5Core2.h>
#include <display.hpp>

/* FSM framework for the menu screen
Loosely based on the "classic" State pattern,
see https://refactoring.guru/design-patterns/state
In each state, actions may be taken on entry, on exit and
on regular updates. The state classes inherit from the virtual base
MenuState and implement the actions.  
 */

// Dimming and blanking times and corresponding labels.
class DimBlankTiming {
  public:
    uint32_t delay_dimming;  // ms before dimming
    uint32_t delay_blanking;  // ms before blanking
    const char *label;
    DimBlankTiming(uint32_t delay_dimming,
                  uint32_t delay_blanking,
                  const char *item_label);
};

class MenuContext;
class Display;

/* Base class for states */
class MenuState {
  protected:
    MenuContext *mcontext_;
  public:
    virtual ~MenuState() {}
    void set_context(MenuContext *context) {
      this->mcontext_ = context;
    }
    virtual void on_entry(MenuContext *context) = 0;
    virtual void on_exit(MenuContext *context) = 0;
    virtual void update(MenuContext *context) = 0;
};


class InitState : public MenuState {
  public:
    void on_entry(MenuContext *context);
    void on_exit(MenuContext *context);
    void update(MenuContext *context);

};

class ConfigState : public MenuState {
  public:
    void on_entry(MenuContext *context);
    void on_exit(MenuContext *context);
    void update(MenuContext *context);

};

/* Menu State Machine Context */
class MenuContext {
  private:
    MenuState *current_state;
    // WiFi parameters
    char myWifiSSID [32] = {'\0'};
    char myIPaddr[16] = {'\0'};
    char configWifiSSID [32] = {'\0'};
    char configWifiPreSharedKey [32] = {'\0'};
    char configIPaddr [20] = {'\0'};

  public:
    int dimblank_idx = 0;
    DimBlankTiming dimblank_choices [5] = {
      {(uint32_t)30*1000, (uint32_t)2*60*1000, "30s /  2m"},
      {(uint32_t)60*1000, (uint32_t)5*60*1000,  " 1m /  5m"},
      {(uint32_t)2*60*1000, (uint32_t)10*60*1000, " 2m / 10m"},
      {(uint32_t)5*60*1000, (uint32_t)30*60*1000, " 5m / 30m"},
      {(uint32_t)0,         (uint32_t)0,         "--- / ---"}  // disable
    };

    // Buttons that appear in the menu screens, statically defined
    ButtonColors onCol = {BLACK, YELLOW, YELLOW};
    ButtonColors offCol = {YELLOW, YELLOW, YELLOW};
    Button button_dimblank{170, 40, 140, 40, false, "", onCol, offCol};
    Button button_config_network{170, 90, 140, 40, false, "", onCol, offCol};
  
    Display *main_display;
    
    MenuContext() : current_state{nullptr} {}

    void goto_state(MenuState *new_state, Display *display);
    void goto_state(MenuState *new_state);

    void update();
};


#endif // __MENU_H__
