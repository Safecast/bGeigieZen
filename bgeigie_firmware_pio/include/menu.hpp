#ifndef __MENU_H__
#define __MENU_H__

#include <M5Core2.h>

/* FSM framework for the menu screen
Loosely based on the "classic" State pattern,
see https://refactoring.guru/design-patterns/state
 */


// Dimming and blanking times and corresponding labels.
class DimBlankTiming {
  public:
    uint32_t delay;  // ms before dimming
    const char *label;
    DimBlankTiming(uint32_t delay_before, const char *item_label);
};

class MenuContext;


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

/* State Machine Context */
class MenuContext {
  private:
    MenuState *current_state;
    // WiFi parameters
    char myWifiSSID [32] = {'\0'};
    char myIPaddr[16] = {'\0'};
    char configWifiSSID [32] = {'\0'};
    char configWifiPreSharedKey [32] = {'\0'};
    char configIPaddr [20] = {'\0'};

    DimBlankTiming dimming_choices [4] = {
      {(uint32_t)30*1000, "30s"},
      {(uint32_t)60*1000, " 1m"},
      {(uint32_t)2*60*1000, " 2m"},
      {(uint32_t)5*60*1000, " 5m"}
    };
    DimBlankTiming blanking_choices [4] = {
      {(uint32_t)2*60*1000, " 2m"},
      {(uint32_t)5*60*1000, " 5m"},
      {(uint32_t)10*60*1000, "10m"},
      {(uint32_t)30*60*1000, "30m"}
    };
    int dimblank_idx = 0;


  public:
    // Buttons that appear in the menu screens, statically defined
    ButtonColors onCol = {BLACK, YELLOW, YELLOW};
    ButtonColors offCol = {RED, YELLOW, YELLOW};
    Button button_dimblank{170, 40, 140, 40, false, "", onCol, offCol};
    
    MenuContext() : current_state{nullptr} {}

    void goto_state(MenuState *new_state);

    void update();
};


#endif // __MENU_H__
