#pragma once
#include <Arduino.h>
#include <stdint.h>

struct ImuSample {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

bool imu_init();
bool imu_read(ImuSample& out);
void imu_calibrate();
