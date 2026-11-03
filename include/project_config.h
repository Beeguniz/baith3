#pragma once

#include <Arduino.h>

namespace config {
constexpr int kLedPins[2] = {13, 12};
constexpr uint32_t kLedPwmHz = 1000;
constexpr uint8_t kLedPwmBits = 8;
constexpr int kMotor1In1 = 5;
constexpr int kMotor1In2 = 17;
constexpr uint32_t kMotorPwmHz = 1000;
constexpr int kSdaPin = 33;
constexpr int kSclPin = 32;
constexpr uint8_t kUnoAddress = 0x08;
constexpr size_t kPacketSize = 4;
constexpr uint32_t kPollIntervalMs = 2000;
constexpr uint32_t kI2cFrequencyHz = 100000;
constexpr uint16_t kI2cTimeoutMs = 50;
constexpr uint32_t kSerialBaud = 115200;
}  // namespace config

// MOTOR2 is strictly out of scope. Never configure or access its pins.
// LED outputs are active HIGH, as confirmed by the user.
