#pragma once
#include <Arduino.h>
#include "config.h"

// Servo PWM setup (ESP32 LEDC).
constexpr uint8_t kServoCount = 7;
constexpr uint8_t kServoPins[kServoCount] = {1, 2, 3, 4, 5, 6, 7};
constexpr uint8_t kServoChannels[kServoCount] = {0, 1, 2, 3, 4, 5, 6};
constexpr uint16_t kServoPwmHz = 50;
constexpr uint8_t kServoPwmBits = 14;
constexpr uint16_t kServoMinUs = 1000;
constexpr uint16_t kServoMaxUs = 2000;
constexpr uint16_t kServoCenterUs = 1500;

// Mixer source IDs: 0..15 = SBUS ch, 16 = roll PID, 17 = pitch PID.
constexpr uint8_t kMixerSrcRollCmd = 16;
constexpr uint8_t kMixerSrcPitchCmd = 17;
// Mixer: index is servo output 0..6, value is mixer source ID.
constexpr uint8_t kServoMixer[kServoCount] = {kMixerSrcRollCmd, kMixerSrcPitchCmd, 2, 3, 4, 5, 6};

void setupServos();
void writeServos();
