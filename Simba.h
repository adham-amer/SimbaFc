#pragma once
#include <sbus.h>
#include <Adafruit_NeoPixel.h>
#include <MadgwickAHRS.h>
#include "config.h"
#include "imu.h"



extern bool USB;


// Receiver
extern bfs::SbusRx sbus_rx;
extern bfs::SbusData data;



// Built-in LED
extern Adafruit_NeoPixel led;
extern bool builtInLedOn;
extern Color color;
void ledOn(Color _color,uint8_t brightness_);
void ledcOn(Color _color,uint8_t brightness_);
void ledOff();
void ledBlink(Color _color,uint8_t brightness_,uint8_t _ticksDivider);
void ledTick();


// Timer flags
extern volatile bool PIDF; // PID flag
extern volatile bool LEDF; // LED flag
extern volatile bool ALTF; // Altimeter flag
extern volatile bool OUTF; // Output flag
extern volatile bool RXDF; // Receiver data in flag
extern volatile bool TXDF; // Receiver data out flag
extern volatile int Ticks;

// Filter

extern Madgwick filter;
extern uint32_t lastTimeUs;
extern float rollOffset;
extern float pitchOffset;
extern float yawOffset;

enum class ControlMode : uint8_t {
  RateIncrement = 0,
  Stabilized = 1,
  AttitudeSwitch = 2
};

struct PidState {
  float kp;
  float ki;
  float kd;
  float integ;
  float prevErr;
  float outMin;
  float outMax;
};

extern ControlMode activeMode;
extern float desiredRollDeg;
extern float desiredPitchDeg;
extern float rollCmd;
extern float pitchCmd;
extern Config gConfig;

float updatePid(PidState& pid, float error, float dt_s);
ControlMode decodeMode(uint16_t ch);
float normalizeSbus(uint16_t ch);
void updateControl(float dt_s);
void applyConfig();

void zeroAttitude() ;

void settleFilter(uint16_t samples, uint16_t delayMs) ;

float convertRawAcceleration(int aRaw);

float convertRawGyro(int gRaw) ;

bool loadConfig();
void saveConfig();


//Commands from Transmitter like Arm, calibrate, etc
extern uint pushCounter;



void setupTimer();
void setLedTickDivider(uint32_t divider);
