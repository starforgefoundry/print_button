#pragma once

#include <Adafruit_NeoPixel.h>

#include <atomic>

#include "indicator-state.h"

// Owns the LED and the button. Runs on its own task so animations and presses
// keep working while the main loop is blocked on HTTP.
class StatusLight {
 public:
  StatusLight();
  void begin();
  void setState(IndicatorState state);
  IndicatorState state() const;
  void identify();
  void acknowledge();
  bool takePress();

 private:
  static void taskEntry(void *self);
  void run();
  void pollButton(uint32_t now);
  uint32_t colorFor(uint32_t now) const;

  Adafruit_NeoPixel pixel_;
  std::atomic<IndicatorState> state_{IndicatorState::SETUP};
  std::atomic<uint32_t> identifyUntil_{0};
  std::atomic<uint32_t> acknowledgeUntil_{0};
  std::atomic<bool> pressed_{false};
  bool buttonDown_ = false;
  uint32_t buttonChangedAt_ = 0;
};
