#include "Simba.h"
#include <Preferences.h>
#include <math.h>


// Receiver
bfs::SbusRx sbus_rx(&Serial1, kSbusRxPin, kSbusTxPin, true, false);
bfs::SbusData data;



// Built-in LED
Adafruit_NeoPixel led(kNumLeds, kLedPin, NEO_GRB + NEO_KHZ800);
Color color=kColorCritical;
uint8_t brightness = 50;


//filter
Madgwick filter;

uint32_t lastTimeUs = 0;
float rollOffset = 0.0f;
float pitchOffset = 0.0f;
float yawOffset = 0.0f;

Config gConfig = {};

ControlMode activeMode = ControlMode::Stabilized;
float desiredRollDeg = 0.0f;
float desiredPitchDeg = 0.0f;
float rollCmd = 0.0f;
float pitchCmd = 0.0f;

// Roll/pitch PID controllers operate on attitude error in degrees and output normalized commands.
static PidState rollPid = {gConfig.rollKp, gConfig.rollKi, gConfig.rollKd, 0.0f, 0.0f, gConfig.pidOutputMin, gConfig.pidOutputMax};
static PidState pitchPid = {gConfig.pitchKp, gConfig.pitchKi, gConfig.pitchKd, 0.0f, 0.0f, gConfig.pidOutputMin, gConfig.pidOutputMax};



extern bool USB = false;


//Commands from Transmitter like Arm, calibrate, etc
uint pushCounter = 0;

//timer inturupts
volatile bool PIDF=false;
volatile bool LEDF=false;
volatile bool RXDF=false;
volatile bool TXDF=false;
volatile bool OUTF=false; // Output flag
volatile bool ALTF=false; // Altimeter flag
volatile int Ticks = 0;
hw_timer_t* timer0 = NULL;

static_assert((kLedTickDivider & (kLedTickDivider - 1)) == 0, "kLedTickDivider must be power of two");
static_assert((servosTickDivider & (servosTickDivider - 1)) == 0, "servosTickDivider must be power of two");
static_assert((altTickDivider & (altTickDivider - 1)) == 0, "altTickDivider must be power of two");
static_assert((rxTickDivider & (rxTickDivider - 1)) == 0, "rxTickDivider must be power of two");
static_assert((txTickDivider & (txTickDivider - 1)) == 0, "txTickDivider must be power of two");
volatile uint32_t kLedTickMask = kLedTickDivider - 1;
constexpr uint32_t kServosTickMask = servosTickDivider - 1;
constexpr uint32_t kAltTickMask = altTickDivider - 1;
constexpr uint32_t kRxDatMask = rxTickDivider - 1;
constexpr uint32_t kTxDatMask = txTickDivider - 1;

void IRAM_ATTR timerCall() {
  PIDF=true;
  Ticks++;
  

  if ((static_cast<uint32_t>(Ticks) & kLedTickMask) == 0) { // slow LED update
    LEDF=true;
  }

  if ((static_cast<uint32_t>(Ticks) & kServosTickMask) == 0) { // servo update
    OUTF=true;
  }
  if ((static_cast<uint32_t>(Ticks) & kAltTickMask) == 0) { // altimeter update
    ALTF=true;
  }
  if ((static_cast<uint32_t>(Ticks) & kRxDatMask) == 0) { // receiver data in
    RXDF=true;
  }
  if ((static_cast<uint32_t>(Ticks) & kTxDatMask) == 0) { // receiver data out
    TXDF=true;
  }
}

void setupTimer() {
  timer0 = timerBegin(kTimerHz); // Timer freq
  timerAttachInterrupt(timer0, &timerCall);
  timerAlarm(timer0, kTimerAlarmTicks, true, 0); // interrupts every 1000 ticks
  filter.begin(kTimerHz / kTimerAlarmTicks); // 1 kHz update rate
  timerStart(timer0);

}

void setLedTickDivider(uint32_t divider) { // divider must be power of two
  if (divider == 0 || (divider & (divider - 1)) != 0) {
    return;
  }
  kLedTickMask = divider - 1;
}

static float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void applyConfig() {
  rollPid.kp = gConfig.rollKp;
  rollPid.ki = gConfig.rollKi;
  rollPid.kd = gConfig.rollKd;
  pitchPid.kp = gConfig.pitchKp;
  pitchPid.ki = gConfig.pitchKi;
  pitchPid.kd = gConfig.pitchKd;

  rollPid.outMin = gConfig.pidOutputMin;
  rollPid.outMax = gConfig.pidOutputMax;
  pitchPid.outMin = gConfig.pidOutputMin;
  pitchPid.outMax = gConfig.pidOutputMax;

  rollPid.integ = 0.0f;
  pitchPid.integ = 0.0f;
  rollPid.prevErr = 0.0f;
  pitchPid.prevErr = 0.0f;
}

void setDefaultConfig() {
  gConfig.rollKp = kRollKp;
  gConfig.rollKi = kRollKi;
  gConfig.rollKd = kRollKd;
  gConfig.pitchKp = kPitchKp;
  gConfig.pitchKi = kPitchKi;
  gConfig.pitchKd = kPitchKd;
  gConfig.maxAttitudeDeg = kMaxAttitudeDeg;
  gConfig.mode1RateDegPerSec = kMode1RateDegPerSec;
  gConfig.mode3StickThreshold = kMode3StickThreshold;
  gConfig.pidOutputMin = kPidOutputMin;
  gConfig.pidOutputMax = kPidOutputMax;
  for (uint8_t i = 0; i < kServoCount; ++i) {
    gConfig.servoMixer[i] = kDefaultServoMixer[i];
  }
}

bool loadConfig() {
  Preferences prefs;
  prefs.begin("simba", true);
  size_t read = prefs.getBytes("cfg", &gConfig, sizeof(gConfig));
  prefs.end();
  if (read != sizeof(gConfig)) {
    setDefaultConfig();
    return false;
  }
  return read == sizeof(gConfig);
}

void saveConfig() {
  Preferences prefs;
  prefs.begin("simba", false);
  prefs.putBytes("cfg", &gConfig, sizeof(gConfig));
  prefs.end();
}

// Convert raw SBUS channel to a normalized stick value in [-1, 1].
float normalizeSbus(uint16_t ch) {
  if (ch >= kSbusCenter) {
    return clampf((static_cast<float>(ch) - kSbusCenter) / (kSbusMax - kSbusCenter), -1.0f, 1.0f);
  }
  return clampf((static_cast<float>(ch) - kSbusCenter) / (kSbusCenter - kSbusMin), -1.0f, 1.0f);
}

// Map a 3-position switch to control modes.
ControlMode decodeMode(uint16_t ch) {
  if (ch < 1200) return ControlMode::RateIncrement;
  if (ch < 1600) return ControlMode::Stabilized;
  return ControlMode::AttitudeSwitch;
}

// PID update with simple anti-windup: clamp the integral so output stays in range.
float updatePid(PidState& pid, float error, float dt_s) {
  if (dt_s <= 0.0f) return 0.0f;
  pid.integ += error * dt_s;
  if (pid.ki > 0.0f) {
    float integMin = pid.outMin / pid.ki;
    float integMax = pid.outMax / pid.ki;
    pid.integ = clampf(pid.integ, integMin, integMax);
  }
  float deriv = (error - pid.prevErr) / dt_s;
  pid.prevErr = error;

  float out = (pid.kp * error) + (pid.ki * pid.integ) + (pid.kd * deriv);
  return clampf(out, pid.outMin, pid.outMax);
}

// Main control update: compute desired attitude from sticks and run PID.
void updateControl(float dt_s) {
  if (dt_s <= 0.0f) return;

  ControlMode newMode = ControlMode::Stabilized;//decodeMode(data.ch[kChMode]);
  if (newMode != activeMode) {
    activeMode = newMode;
    // Reset integrators when switching modes to avoid sudden output jumps.
    rollPid.integ = 0.0f;
    pitchPid.integ = 0.0f;
    rollPid.prevErr = 0.0f;
    pitchPid.prevErr = 0.0f;
    // Start setpoints from current attitude for smooth mode transitions.
    desiredRollDeg = filter.getRoll() - rollOffset;
    desiredPitchDeg = filter.getPitch() - pitchOffset;
  }

  float stickRoll = normalizeSbus(data.ch[kChRoll]);
  float stickPitch = normalizeSbus(data.ch[kChPitch]);
  float rollDeg = filter.getRoll() - rollOffset;
  float pitchDeg = filter.getPitch() - pitchOffset;

  // Mode behavior:
  // - RateIncrement: sticks add to desired attitude (rate-based).
  // - Stabilized: sticks command absolute attitude directly.
  // - AttitudeSwitch: stabilized until stick exceeds threshold, then rate.
  switch (activeMode) {
    case ControlMode::RateIncrement:
      desiredRollDeg += stickRoll * gConfig.mode1RateDegPerSec * dt_s;
      desiredPitchDeg += stickPitch * gConfig.mode1RateDegPerSec * dt_s;
      break;
    case ControlMode::Stabilized:
      desiredRollDeg = stickRoll * gConfig.maxAttitudeDeg;
      desiredPitchDeg = stickPitch * gConfig.maxAttitudeDeg;
      break;
    case ControlMode::AttitudeSwitch:
      if (fabsf(stickRoll) > gConfig.mode3StickThreshold || fabsf(stickPitch) > gConfig.mode3StickThreshold) {
        desiredRollDeg += stickRoll * gConfig.mode1RateDegPerSec * dt_s;
        desiredPitchDeg += stickPitch * gConfig.mode1RateDegPerSec * dt_s;
      } else {
        desiredRollDeg = stickRoll * gConfig.maxAttitudeDeg;
        desiredPitchDeg = stickPitch * gConfig.maxAttitudeDeg;
      }
      break;
  }

  desiredRollDeg = clampf(desiredRollDeg, -gConfig.maxAttitudeDeg, gConfig.maxAttitudeDeg);
  desiredPitchDeg = clampf(desiredPitchDeg, -gConfig.maxAttitudeDeg, gConfig.maxAttitudeDeg);

  float rollErr = desiredRollDeg - rollDeg;
  float pitchErr = desiredPitchDeg - pitchDeg;
  rollCmd = updatePid(rollPid, rollErr, dt_s);
  pitchCmd = updatePid(pitchPid, pitchErr, dt_s);
}


bool begun =false;
bool LedOn = false;
bool _ledblink = false;
void ledTick() {
	if (!begun) {
		led.begin();
    begun=true;
	}

  if (_ledblink) {
    if (LedOn) {
      ledOff();
      LedOn = false;
    } else {
      ledOn(color, brightness);
      LedOn = true;
    }
  }
  	
}

void ledBlink(Color _color,uint8_t brightness_,uint8_t _ticksDivider) {
  setLedTickDivider(_ticksDivider);
  color=_color;
  brightness=brightness_;
  _ledblink = true;
}
void ledOn(Color _color,uint8_t brightness_) {
  color=_color;
	if (!begun) {
		led.begin();
    begun=true;
	}
   brightness=brightness_;
  	led.setBrightness(brightness);
  	led.setPixelColor(0, led.Color(color.r, color.g, color.b)); 
  	led.show();
}
void ledcOn(Color _color,uint8_t brightness_) {
  color=_color;
  _ledblink = false;
	if (!begun) {
		led.begin();
    begun=true;
	}
   brightness=brightness_;
  	led.setBrightness(brightness);
  	led.setPixelColor(0, led.Color(color.r, color.g, color.b)); 
  	led.show();
}
void ledOff() {
	if (!begun) {
		led.begin();
    begun=true;
	}
  	led.setBrightness(0);
  	led.setPixelColor(0, led.Color(color.r, color.g, color.b)); 
  	led.show();
}


void zeroAttitude() {
  rollOffset = filter.getRoll();
  pitchOffset = filter.getPitch();
  yawOffset = filter.getYaw();
}

void settleFilter(uint16_t samples, uint16_t delayMs) {
  ImuSample tmp;
  for (uint16_t i = 0; i < samples; ++i) {
    imu_read(tmp);
    filter.updateIMU(
      convertRawGyro(tmp.gx), convertRawGyro(tmp.gy), convertRawGyro(tmp.gz),
      convertRawAcceleration(tmp.ax), convertRawAcceleration(tmp.ay), convertRawAcceleration(tmp.az)
    );
    delay(delayMs);
  }
}

float convertRawAcceleration(int aRaw) {
  // since we are using 2G range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767
  
  float a = (aRaw * 2.0) / 32768.0;
  return a;
}

float convertRawGyro(int gRaw) {
  // since we are using 250 degrees/seconds range
  // -250 maps to a raw value of -32768
  // +250 maps to a raw value of 32767
  
  float g = (gRaw * 2000.0) / 32768.0;
  return g;
}
