#pragma once

enum class IndicatorState {
  SETUP,
  UNCONFIGURED,
  OFFLINE,
  ERROR,
  BUSY,
  PLATE_READY,
  IDLE,
};

inline const char *indicatorStateName(IndicatorState state) {
  switch (state) {
    case IndicatorState::SETUP:
      return "setup";
    case IndicatorState::UNCONFIGURED:
      return "unconfigured";
    case IndicatorState::OFFLINE:
      return "offline";
    case IndicatorState::ERROR:
      return "error";
    case IndicatorState::BUSY:
      return "busy";
    case IndicatorState::PLATE_READY:
      return "plateReady";
    case IndicatorState::IDLE:
      return "idle";
  }
  return "unknown";
}
