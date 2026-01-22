#pragma once
#include <Arduino.h>
#include <SPI.h>

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// ====== BOARD PINS ======
constexpr uint8_t kSbusRxPin = 8;
constexpr uint8_t kSbusTxPin = 9;

constexpr uint8_t kLedPin = 48;
constexpr uint8_t kNumLeds = 1;

// ====== STATUS COLORS ======
constexpr Color kColorCritical    = {255, 0, 0};
constexpr Color kColorInitializing = {0, 0, 255};
constexpr Color kColorCalibrating = {0, 0, 255};
constexpr Color kColorSafe        = {0, 200, 0};
constexpr Color kColorData        = {200, 200, 0};
constexpr Color kColorWarning     = {255, 75, 0};

// If SBUS and IMU share pins on your board, update these to avoid conflicts.
constexpr uint8_t kImuSckPin  = 13;
constexpr uint8_t kImuMisoPin = 12;
constexpr uint8_t kImuMosiPin = 11;
constexpr uint8_t kImuCsPin   = 10;

// ====== TIMING ======
constexpr uint32_t kTimerHz = 1000000;
constexpr uint32_t kTimerAlarmTicks = 1000; //1 kHz loop use 1000, for 800hz use 1250
constexpr uint8_t kLedTickDivider = 256;     // slow LED update, time is calculated as (kLedTickDivider / kTimerHz) seconds
constexpr uint8_t servosTickDivider = 16;  // servo update rate divider
constexpr uint8_t altTickDivider = 32; // altimeter update rate divider
constexpr uint8_t rxTickDivider = 32; // receiver in update rate divide
constexpr uint8_t txTickDivider = 32; // receiver out update rate divider
// ====== IMU SPI ======
constexpr uint32_t kImuSpiHz = 10000000; // BMI160 supports up to 10 MHz
constexpr uint8_t kImuSpiMode = SPI_MODE0;
