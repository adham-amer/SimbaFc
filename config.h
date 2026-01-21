#pragma once
#include <Arduino.h>
#include <SPI.h>

// ====== BOARD PINS ======
constexpr uint8_t kSbusRxPin = 10;
constexpr uint8_t kSbusTxPin = 11;

constexpr uint8_t kLedPin = 48;
constexpr uint8_t kNumLeds = 1;

// If SBUS and IMU share pins on your board, update these to avoid conflicts.
constexpr uint8_t kImuSckPin  = 13;
constexpr uint8_t kImuMisoPin = 12;
constexpr uint8_t kImuMosiPin = 11;
constexpr uint8_t kImuCsPin   = 10;

// ====== TIMING ======
constexpr uint32_t kTimerHz = 1000000;
constexpr uint32_t kTimerAlarmTicks = 1000; //1 kHz loop use 1000, for 800hz use 1250
constexpr uint8_t kLedTickDivider = 20;     // slow LED update, time is calculated as (kLedTickDivider / kTimerHz) seconds

// ====== IMU SPI ======
constexpr uint32_t kImuSpiHz = 10000000; // BMI160 supports up to 10 MHz
constexpr uint8_t kImuSpiMode = SPI_MODE0;
