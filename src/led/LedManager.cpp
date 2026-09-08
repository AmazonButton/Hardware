#include "LedManager.h"

void LedManager::init() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  setColor(LED_OFF);
}

void LedManager::setColor(LedColor color) {
  // Schematic D1 is Common Anode tied to +3.3V
  // Active LOW logic: LOW (0V) = LED ON, HIGH (3.3V) = LED OFF
  switch (color) {
    case LED_BLUE:
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, LOW);
      break;

    case LED_GREEN:
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, HIGH);
      break;

    case LED_RED:
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, HIGH);
      break;

    case LED_YELLOW: // Red + Green
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, HIGH);
      break;

    case LED_PURPLE: // Red + Blue
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, LOW);
      break;

    case LED_WHITE: // Red + Green + Blue
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, LOW);
      break;

    case LED_OFF:
    default:
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, HIGH);
      break;
  }
}

void LedManager::blink(LedColor color, int times, int intervalMs) {
  for (int i = 0; i < times; i++) {
    setColor(color);
    delay(intervalMs);
    setColor(LED_OFF);
    if (i < times - 1) delay(intervalMs);
  }
}
