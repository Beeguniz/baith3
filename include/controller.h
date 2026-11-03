#pragma once
#include <stdint.h>

// Pure logic only: this file has no GPIO or PWM operations.
enum class State { Stopped, Running, Fault };
struct Channel {
  const char* name;
  State state = State::Stopped;
  uint32_t started = 0;
  uint32_t revision = 0;
  unsigned duty = 50;
  const char* reason = "BOOT";
};
constexpr uint32_t kRunLimitMs = 20000;

inline void stopChannel(Channel& c, const char* reason) {
  c.state = State::Stopped;
  c.reason = reason;
  ++c.revision;
}
inline void faultChannel(Channel& c, const char* reason) {
  c.state = State::Fault;
  c.reason = reason;
  ++c.revision;
}
inline void updateChannel(Channel& c, uint32_t now, bool sensorValid) {
  if (c.state == State::Fault) {
    stopChannel(c, c.reason);
    return;
  }
  if (c.state != State::Running) return;
  if (!sensorValid) faultChannel(c, "SENSOR_ERROR");
  else if (uint32_t(now - c.started) >= kRunLimitMs)
    faultChannel(c, "TIMEOUT");
}
inline bool startChannel(Channel& c, uint32_t now, bool sensorValid,
                         uint32_t expectedRevision) {
  if (!sensorValid || c.state == State::Fault ||
      expectedRevision != c.revision) return false;
  if (c.state == State::Running) return true; // Never extend run time.
  c.state = State::Running;
  c.started = now;
  c.reason = "START";
  ++c.revision;
  return true;
}
