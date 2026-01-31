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

// ====== CONTROL / SBUS ======
constexpr uint16_t kSbusMin = 172;
constexpr uint16_t kSbusMax = 1811;
constexpr uint16_t kSbusCenter = 992;
constexpr uint8_t kChRoll = 3;
constexpr uint8_t kChPitch = 2;
constexpr uint8_t kChYaw = 1;
constexpr uint8_t kChThrottle = 0;
constexpr uint8_t kChMode = 8; // 3-position switch

// Attitude control tuning (degrees)
constexpr float kMaxAttitudeDeg = 35.0f;
constexpr float kMode1RateDegPerSec = 60.0f;
constexpr float kMode3StickThreshold = 0.75f; // normalized stick, 0..1

// PID gains (roll/pitch)
constexpr float kRollKp = 4.0f;
constexpr float kRollKi = 0.8f;
constexpr float kRollKd = 0.08f;
constexpr float kPitchKp = 4.0f;
constexpr float kPitchKi = 0.8f;
constexpr float kPitchKd = 0.08f;
constexpr float kPidOutputMin = -90.0f;
constexpr float kPidOutputMax = 90.0f;

// ====== SERVO MIXER ======
constexpr uint8_t kServoCount = 7;
// Mixer source IDs: 0..15 = SBUS ch, 16 = roll PID, 17 = pitch PID.
constexpr uint8_t kMixerSrcRollCmd = 16;
constexpr uint8_t kMixerSrcPitchCmd = 17;
// Default mixer: index is servo output 0..6, value is mixer source ID.
constexpr uint8_t kDefaultServoMixer[kServoCount] = {
  kMixerSrcRollCmd, kMixerSrcPitchCmd, 2, 3, 4, 5, 6
};

struct Config {
  float rollKp;
  float rollKi;
  float rollKd;
  float pitchKp;
  float pitchKi;
  float pitchKd;
  float maxAttitudeDeg;
  float mode1RateDegPerSec;
  float mode3StickThreshold;
  float pidOutputMin;
  float pidOutputMax;
  uint8_t servoMixer[kServoCount];
};
