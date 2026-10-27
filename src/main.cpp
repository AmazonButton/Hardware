#include <Arduino.h>
#include "core/StateMachine.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("SMART ORDER ESP32-S3 OK");

  StateMachine::init();
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.println("Running...");
  }

  StateMachine::loop();
}
