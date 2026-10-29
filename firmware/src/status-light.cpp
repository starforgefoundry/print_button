#include "status-light.h"

#include <math.h>

namespace {
constexpr uint8_t LED_PIN = 35;
constexpr uint8_t BUTTON_PIN = 41;
constexpr uint8_t LED_COUNT = 1;
constexpr uint8_t BRIGHTNESS = 80;
constexpr uint32_t FRAME_MS = 10;
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t IDENTIFY_MS = 10000;
constexpr uint32_t IDENTIFY_BLINK_MS = 250;
constexpr uint32_t ACKNOWLEDGE_MS = 300;
constexpr uint32_t PULSE_PERIOD_MS = 2000;
constexpr uint32_t ERROR_BLINK_MS = 500;
constexpr uint32_t TASK_STACK = 4096;
constexpr UBaseType_t TASK_PRIORITY = 2;

constexpr uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) { return (uint32_t(r) << 16) | (uint32_t(g) << 8) | b; }

constexpr uint32_t WHITE = rgb(255, 255, 255);
constexpr uint32_t RED = rgb(255, 0, 0);
constexpr uint32_t YELLOW = rgb(255, 160, 0);
constexpr uint32_t GREEN = rgb(0, 255, 0);
constexpr uint32_t BLUE = rgb(0, 0, 255);
constexpr uint32_t CYAN = rgb(0, 200, 255);
constexpr uint32_t MAGENTA = rgb(255, 0, 255);
constexpr uint32_t DIM_MAGENTA = rgb(40, 0, 40);
constexpr uint32_t OFF = 0;

uint32_t scale(uint32_t color, float factor) {
  auto channel = [&](int shift) { return uint32_t(((color >> shift) & 0xFF) * factor) << shift; };
  return channel(16) | channel(8) | channel(0);
}

uint32_t pulse(uint32_t color, uint32_t now) {
  float phase = float(now % PULSE_PERIOD_MS) / PULSE_PERIOD_MS;
  return scale(color, 0.1f + 0.9f * (0.5f - 0.5f * cosf(phase * 2 * PI)));
}

bool blinkOn(uint32_t now, uint32_t halfPeriod) { return (now / halfPeriod) % 2 == 0; }

bool before(uint32_t now, uint32_t deadline) { return int32_t(deadline - now) > 0; }
}  // namespace

StatusLight::StatusLight() : pixel_(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800) {}

void StatusLight::begin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pixel_.begin();
  pixel_.setBrightness(BRIGHTNESS);
  xTaskCreate(taskEntry, "statusLight", TASK_STACK, this, TASK_PRIORITY, nullptr);
}

void StatusLight::setState(IndicatorState state) { state_ = state; }

IndicatorState StatusLight::state() const { return state_; }

void StatusLight::identify() { identifyUntil_ = millis() + IDENTIFY_MS; }

void StatusLight::acknowledge() { acknowledgeUntil_ = millis() + ACKNOWLEDGE_MS; }

bool StatusLight::takePress() { return pressed_.exchange(false); }

void StatusLight::taskEntry(void *self) { static_cast<StatusLight *>(self)->run(); }

void StatusLight::run() {
  for (;;) {
    uint32_t now = millis();
    pollButton(now);
    pixel_.setPixelColor(0, colorFor(now));
    pixel_.show();
    vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
  }
}

void StatusLight::pollButton(uint32_t now) {
  bool down = digitalRead(BUTTON_PIN) == LOW;
  if (down == buttonDown_) {
    buttonChangedAt_ = now;
    return;
  }
  if (now - buttonChangedAt_ < DEBOUNCE_MS) return;
  buttonDown_ = down;
  if (down) pressed_ = true;
}

uint32_t StatusLight::colorFor(uint32_t now) const {
  if (before(now, acknowledgeUntil_)) return WHITE;
  if (before(now, identifyUntil_)) return blinkOn(now, IDENTIFY_BLINK_MS) ? WHITE : OFF;

  switch (state_.load()) {
    case IndicatorState::SETUP:
      return pulse(BLUE, now);
    case IndicatorState::UNCONFIGURED:
      return pulse(CYAN, now);
    case IndicatorState::OFFLINE:
      return DIM_MAGENTA;
    case IndicatorState::ERROR:
      return blinkOn(now, ERROR_BLINK_MS) ? MAGENTA : OFF;
    case IndicatorState::BUSY:
      return RED;
    case IndicatorState::PLATE_READY:
      return YELLOW;
    case IndicatorState::IDLE:
      return GREEN;
  }
  return OFF;
}
