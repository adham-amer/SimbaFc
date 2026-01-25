#include "mixer.h"
#include "Simba.h"
#include <esp_arduino_version.h>

namespace {
float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

uint32_t usToDuty(uint16_t us) {
  const uint32_t maxDuty = (1u << kServoPwmBits) - 1u;
  const uint32_t periodUs = 1000000u / kServoPwmHz;
  return static_cast<uint32_t>((static_cast<uint64_t>(us) * maxDuty) / periodUs);
}

uint16_t sbusToUs(uint16_t ch) {
  float t = (static_cast<float>(ch) - kSbusMin) / (kSbusMax - kSbusMin);
  t = clampf(t, 0.0f, 1.0f);
  return static_cast<uint16_t>(kServoMinUs + t * (kServoMaxUs - kServoMinUs));
}

uint16_t cmdToUs(float cmd) {
  const float span = gConfig.pidOutputMax - gConfig.pidOutputMin;
  if (span <= 0.0f) {
    return kServoCenterUs;
  }
  float t = (cmd - gConfig.pidOutputMin) / span;
  t = clampf(t, 0.0f, 1.0f);
  return static_cast<uint16_t>(kServoMinUs + t * (kServoMaxUs - kServoMinUs));
}
}

void setupServos() {
  for (uint8_t i = 0; i < kServoCount; ++i) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(kServoPins[i], kServoPwmHz, kServoPwmBits);
    ledcWrite(kServoPins[i], usToDuty(kServoCenterUs));
#else
    ledcSetup(kServoChannels[i], kServoPwmHz, kServoPwmBits);
    ledcAttachPin(kServoPins[i], kServoChannels[i]);
    ledcWrite(kServoChannels[i], usToDuty(kServoCenterUs));
#endif
  }
}

void writeServos() {
  for (uint8_t i = 0; i < kServoCount; ++i) {
    const uint8_t src = kServoMixer[i];
    uint16_t us = kServoCenterUs;
    if (src < 16) {
      us = sbusToUs(data.ch[src]);
    } else if (src == kMixerSrcRollCmd) {
      us = cmdToUs(rollCmd);
    } else if (src == kMixerSrcPitchCmd) {
      us = cmdToUs(pitchCmd);
    }
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(kServoPins[i], usToDuty(us));
#else
    ledcWrite(kServoChannels[i], usToDuty(us));
#endif
  }
}
