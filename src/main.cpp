#include <Arduino.h>
#include "core/StateMachine.h"

void setup() {
  StateMachine::init();
}

void loop() {
  StateMachine::loop();
}
