#pragma once
#include <Arduino.h>
#include "imu.h"

struct Attitude {
  float roll_deg;
  float pitch_deg;
  float yaw_deg;
};

class ImuFusion {
 public:
  // Initialize filter defaults for BMI160 and reset state.
  ImuFusion();

  // Clear attitude state; next update re-initializes from accel tilt.
  void reset();
  // Set steady-state gyro bias (dps) to reduce drift.
  void setGyroBiasDps(float bx, float by, float bz);
  // Set accelerometer scale factor (LSB per g).
  void setAccelScale(float lsb_per_g);
  // Set gyroscope scale factor (LSB per dps).
  void setGyroScale(float lsb_per_dps);
  // Set complementary filter time constant (seconds).
  void setTimeConstant(float tau_s);
  // Limit accel correction to near-1g conditions.
  void setAccelTrustRange(float min_g, float max_g);

  // Fuse one IMU sample into roll/pitch/yaw using dt_s (seconds).
  Attitude update(const ImuSample& s, float dt_s);

 private:
  float accel_lsb_per_g_;
  float gyro_lsb_per_dps_;
  float tau_s_;
  float min_g_;
  float max_g_;

  float bias_gx_;
  float bias_gy_;
  float bias_gz_;

  float roll_deg_;
  float pitch_deg_;
  float yaw_deg_;
  bool initialized_;
};
