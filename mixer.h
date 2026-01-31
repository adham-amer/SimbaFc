#pragma once
#include <Arduino.h>
#include "config.h"

// Servo PWM setup (ESP32 LEDC).
constexpr uint8_t kServoPins[kServoCount] = {1, 2, 3, 4, 5, 6, 7};
constexpr uint8_t kServoChannels[kServoCount] = {0, 1, 2, 3, 4, 5, 6};
constexpr uint16_t kServoPwmHz = 50;
constexpr uint8_t kServoPwmBits = 14;
constexpr uint16_t kServoMinUs = 1000;
constexpr uint16_t kServoMaxUs = 2000;
constexpr uint16_t kServoCenterUs = 1500;

void setupServos();
void writeServos();
