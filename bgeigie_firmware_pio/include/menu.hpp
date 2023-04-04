#ifndef __MENU_H__
#define __MENU_H__

#include <M5Core2.h>

/* FSM framework for the menu screen
Loosely based on the "classic" State pattern,
see https://refactoring.guru/design-patterns/state
 */

//#include <display.hpp>

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

  public:
    MenuContext() : current_state(nullptr) {}

    void goto_state(MenuState *new_state) {
      if(this->current_state != nullptr) {
        /*DEBUG*/ Serial.println("goto_state: current_state != nullptr, executing on_exit()");
        this->current_state->on_exit(this);
      }
      this->current_state = new_state;
      /*DEBUG*/ Serial.println("goto_state: executing on_entry()");
      this->current_state->on_entry(this);
    }

    void update() {
      if(this->current_state != nullptr) {
        this->current_state->update(this);
      }
    }
};


class InitState : public MenuState {
  public:
    void on_entry(MenuContext *context);
    void on_exit(MenuContext *context);
    void update(MenuContext *context);

};


#endif // __MENU_H__
