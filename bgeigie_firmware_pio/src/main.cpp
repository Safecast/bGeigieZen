// Test code from Average CPM count with sliding window for bGeigeiRaku. Code
// taken from bGeigieNano and Pointcast.

#include <M5Core2.h> // #include <M5Stack.h>
#include <fsm_context.hpp>

Context context;

void setup() {
  context.begin();
}

void loop() { context.loop(); }
