#include "SimbaFilter.h"
#include <math.h>

// Wrap degrees to [-180, 180] to keep angles bounded.
static inline float wrapDeg(float a)
{
  while (a > 180.0f) a -= 360.0f;
  while (a < -180.0f) a += 360.0f;
  return a;
}

// Construct filter with BMI160 defaults and zeroed attitude.
ImuFusion::ImuFusion()
{
  accel_lsb_per_g_ = 16384.0f; // BMI160 +/-2g
  gyro_lsb_per_dps_ = 16.4f;   // BMI160 +/-2000 dps
  tau_s_ = 0.5f;
  min_g_ = 0.8f;
  max_g_ = 1.2f;
  bias_gx_ = 0.0f;
  bias_gy_ = 0.0f;
  bias_gz_ = 0.0f;
  roll_deg_ = 0.0f;
  pitch_deg_ = 0.0f;
  yaw_deg_ = 0.0f;
  initialized_ = false;
}

// Reset attitude state; next update re-initializes from accel.
void ImuFusion::reset()
{
  roll_deg_ = 0.0f;
  pitch_deg_ = 0.0f;
  yaw_deg_ = 0.0f;
  initialized_ = false;
}

// Apply gyro bias (dps) for drift compensation.
void ImuFusion::setGyroBiasDps(float bx, float by, float bz)
{
  bias_gx_ = bx;
  bias_gy_ = by;
  bias_gz_ = bz;
}

// Update accel conversion (LSB per g).
void ImuFusion::setAccelScale(float lsb_per_g)
{
  if (lsb_per_g > 0.0f) accel_lsb_per_g_ = lsb_per_g;
}

// Update gyro conversion (LSB per dps).
void ImuFusion::setGyroScale(float lsb_per_dps)
{
  if (lsb_per_dps > 0.0f) gyro_lsb_per_dps_ = lsb_per_dps;
}

// Set complementary filter time constant (seconds).
void ImuFusion::setTimeConstant(float tau_s)
{
  if (tau_s > 0.0f) tau_s_ = tau_s;
}

// Set accel magnitude window for trusting tilt correction.
void ImuFusion::setAccelTrustRange(float min_g, float max_g)
{
  if (min_g > 0.0f && max_g > min_g) {
    min_g_ = min_g;
    max_g_ = max_g;
  }
}

// Fuse gyro and accel to compute roll/pitch/yaw for this sample.
Attitude ImuFusion::update(const ImuSample& s, float dt_s)
{
  // Convert raw accel to g.
  const float ax = (float)s.ax / accel_lsb_per_g_;
  const float ay = (float)s.ay / accel_lsb_per_g_;
  const float az = (float)s.az / accel_lsb_per_g_;

  // Convert raw gyro to dps and remove bias.
  const float gx = (float)s.gx / gyro_lsb_per_dps_ - bias_gx_;
  const float gy = (float)s.gy / gyro_lsb_per_dps_ - bias_gy_;
  const float gz = (float)s.gz / gyro_lsb_per_dps_ - bias_gz_;

  // Use accel only when magnitude is near 1g (reduces correction during maneuvers).
  const float acc_mag = sqrtf(ax * ax + ay * ay + az * az);
  const bool use_accel = (acc_mag > min_g_) && (acc_mag < max_g_);

  // Compute tilt from accel for roll/pitch reference.
  const float roll_acc = atan2f(ay, az) * RAD_TO_DEG;
  const float pitch_acc = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

  // Initialize attitude from accel on first sample to avoid a jump.
  if (!initialized_) {
    roll_deg_ = roll_acc;
    pitch_deg_ = pitch_acc;
    yaw_deg_ = 0.0f;
    initialized_ = true;
    return {roll_deg_, pitch_deg_, yaw_deg_};
  }

  // Guard dt and compute complementary filter weight.
  if (dt_s <= 0.0f) dt_s = 0.001f;
  const float alpha = tau_s_ / (tau_s_ + dt_s);

  // Integrate gyro for short-term attitude propagation.
  const float roll_gyro = roll_deg_ + gx * dt_s;
  const float pitch_gyro = pitch_deg_ + gy * dt_s;
  const float yaw_gyro = yaw_deg_ + gz * dt_s;

  // Blend gyro and accel for roll/pitch; yaw is gyro-only.
  if (use_accel) {
    roll_deg_ = alpha * roll_gyro + (1.0f - alpha) * roll_acc;
    pitch_deg_ = alpha * pitch_gyro + (1.0f - alpha) * pitch_acc;
  } else {
    roll_deg_ = roll_gyro;
    pitch_deg_ = pitch_gyro;
  }

  // Keep angles bounded for numerical stability.
  yaw_deg_ = wrapDeg(yaw_gyro);
  roll_deg_ = wrapDeg(roll_deg_);
  pitch_deg_ = wrapDeg(pitch_deg_);

  return {roll_deg_, pitch_deg_, yaw_deg_};
}
