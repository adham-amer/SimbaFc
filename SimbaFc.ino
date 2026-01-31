#include "Simba.h"
#include "mixer.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
namespace {
constexpr size_t kLineBufLen = 160;
char lineBuf[kLineBufLen];
size_t lineLen = 0;

float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void printConfig() {
  Serial.print("CFG ");
  Serial.print("rollKp="); Serial.print(gConfig.rollKp, 4);
  Serial.print(" rollKi="); Serial.print(gConfig.rollKi, 4);
  Serial.print(" rollKd="); Serial.print(gConfig.rollKd, 4);
  Serial.print(" pitchKp="); Serial.print(gConfig.pitchKp, 4);
  Serial.print(" pitchKi="); Serial.print(gConfig.pitchKi, 4);
  Serial.print(" pitchKd="); Serial.print(gConfig.pitchKd, 4);
  Serial.print(" maxAttitudeDeg="); Serial.print(gConfig.maxAttitudeDeg, 2);
  Serial.print(" mode1RateDegPerSec="); Serial.print(gConfig.mode1RateDegPerSec, 2);
  Serial.print(" mode3StickThreshold="); Serial.print(gConfig.mode3StickThreshold, 3);
  Serial.print(" pidOutputMin="); Serial.print(gConfig.pidOutputMin, 2);
  Serial.print(" pidOutputMax="); Serial.print(gConfig.pidOutputMax, 2);
  Serial.print(" mixer=");
  for (uint8_t i = 0; i < kServoCount; ++i) {
    if (i > 0) Serial.print(",");
    const uint8_t src = gConfig.servoMixer[i];
    if (src == kMixerSrcRollCmd) {
      Serial.print("roll");
    } else if (src == kMixerSrcPitchCmd) {
      Serial.print("pitch");
    } else {
      Serial.print("ch");
      Serial.print(src);
    }
  }
  Serial.println();
}

bool parseFloat(const char* s, float* out) {
  if (!s || !*s) return false;
  *out = static_cast<float>(atof(s));
  return true;
}

bool setMixerEntry(uint8_t index, const char* value) {
  if (index >= kServoCount || value == nullptr) return false;
  if (strcasecmp(value, "roll") == 0) {
    gConfig.servoMixer[index] = kMixerSrcRollCmd;
    return true;
  }
  if (strcasecmp(value, "pitch") == 0) {
    gConfig.servoMixer[index] = kMixerSrcPitchCmd;
    return true;
  }
  if (value[0] == 'c' || value[0] == 'C') {
    if (value[1] == 'h' || value[1] == 'H') {
      int ch = atoi(value + 2);
      if (ch >= 0 && ch < 16) {
        gConfig.servoMixer[index] = static_cast<uint8_t>(ch);
        return true;
      }
    }
  }
  return false;
}

void handleSetCommand(char* args) {
  bool changed = false;
  char* tok = strtok(args, " ");
  while (tok) {
    char* eq = strchr(tok, '=');
    if (eq) {
      *eq = '\0';
      const char* key = tok;
      const char* val = eq + 1;
      float f = 0.0f;
      if (strcmp(key, "rollKp") == 0 && parseFloat(val, &f)) { gConfig.rollKp = f; changed = true; }
      else if (strcmp(key, "rollKi") == 0 && parseFloat(val, &f)) { gConfig.rollKi = f; changed = true; }
      else if (strcmp(key, "rollKd") == 0 && parseFloat(val, &f)) { gConfig.rollKd = f; changed = true; }
      else if (strcmp(key, "pitchKp") == 0 && parseFloat(val, &f)) { gConfig.pitchKp = f; changed = true; }
      else if (strcmp(key, "pitchKi") == 0 && parseFloat(val, &f)) { gConfig.pitchKi = f; changed = true; }
      else if (strcmp(key, "pitchKd") == 0 && parseFloat(val, &f)) { gConfig.pitchKd = f; changed = true; }
      else if (strcmp(key, "maxAttitudeDeg") == 0 && parseFloat(val, &f)) { gConfig.maxAttitudeDeg = f; changed = true; }
      else if (strcmp(key, "mode1RateDegPerSec") == 0 && parseFloat(val, &f)) { gConfig.mode1RateDegPerSec = f; changed = true; }
      else if (strcmp(key, "mode3StickThreshold") == 0 && parseFloat(val, &f)) { gConfig.mode3StickThreshold = f; changed = true; }
      else if (strcmp(key, "pidOutputMin") == 0 && parseFloat(val, &f)) { gConfig.pidOutputMin = f; changed = true; }
      else if (strcmp(key, "pidOutputMax") == 0 && parseFloat(val, &f)) { gConfig.pidOutputMax = f; changed = true; }
    }
    tok = strtok(nullptr, " ");
  }

  gConfig.maxAttitudeDeg = clampf(gConfig.maxAttitudeDeg, 1.0f, 90.0f);
  gConfig.mode1RateDegPerSec = clampf(gConfig.mode1RateDegPerSec, 1.0f, 360.0f);
  gConfig.mode3StickThreshold = clampf(gConfig.mode3StickThreshold, 0.0f, 1.0f);
  if (gConfig.pidOutputMin > gConfig.pidOutputMax) {
    float tmp = gConfig.pidOutputMin;
    gConfig.pidOutputMin = gConfig.pidOutputMax;
    gConfig.pidOutputMax = tmp;
  }

  if (changed) {
    applyConfig();
    saveConfig();
    Serial.println("OK");
  } else {
    Serial.println("ERR no changes");
  }
}

void handleMixCommand(char* args) {
  bool changed = false;
  char* tok = strtok(args, " ");
  while (tok) {
    char* eq = strchr(tok, '=');
    if (eq) {
      *eq = '\0';
      int idx = atoi(tok);
      if (idx >= 0 && idx < kServoCount) {
        if (setMixerEntry(static_cast<uint8_t>(idx), eq + 1)) {
          changed = true;
        }
      }
    }
    tok = strtok(nullptr, " ");
  }

  if (changed) {
    applyConfig();
    saveConfig();
    Serial.println("OK");
  } else {
    Serial.println("ERR mix");
  }
}

void handleLine(char* line) {
  while (*line == ' ') ++line;
  if (*line == '\0') return;

  if (strcasecmp(line, "GET") == 0) {
    printConfig();
    return;
  }
  if (strcasecmp(line, "HELP") == 0) {
    Serial.println("CMD GET");
    Serial.println("CMD SET key=val ...");
    Serial.println("CMD MIX index=roll|pitch|chN ...");
    return;
  }

  char* space = strchr(line, ' ');
  if (!space) {
    Serial.println("ERR cmd");
    return;
  }
  *space = '\0';
  char* args = space + 1;
  if (strcasecmp(line, "SET") == 0) {
    handleSetCommand(args);
  } else if (strcasecmp(line, "MIX") == 0) {
    handleMixCommand(args);
  } else {
    Serial.println("ERR cmd");
  }
}

void pollSerialConfig() {
  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      lineBuf[lineLen] = '\0';
      handleLine(lineBuf);
      lineLen = 0;
      continue;
    }
    if (lineLen + 1 < kLineBufLen) {
      lineBuf[lineLen++] = static_cast<char>(c);
    } else {
      lineLen = 0;
      Serial.println("ERR line");
    }
  }
}
}  // namespace

void setup() {
  setupTimer();

  ledcOn(kColorInitializing, 50);
  Serial.begin(115200);
  int x =0;
  while (!Serial) {
    ledTick();
    delay(10);
    x++;
    if (x>100) break;
  }

  if (x < 100) {
    USB = true;
  }

  delay(100);

  ledcOn(kColorInitializing, 10);
  imu_init();


  ledcOn(kColorWarning, 50);
  imu_calibrate();
  settleFilter(200, 1);
  zeroAttitude();

  setupServos();
  setDefaultConfig();
  loadConfig();
  applyConfig();
  
  sbus_rx.Begin();
  lastTimeUs = micros();
}

float dt_s = 0.0f;
ImuSample s;
 
void loop() {

  if (PIDF) { //1khz loop
   PIDF = false;
    //calc time and dt
    uint32_t nowUs = micros();
    dt_s = (nowUs - lastTimeUs) * 1e-6f;
    lastTimeUs = nowUs;
    imu_read(s);
    filter.updateIMU(convertRawGyro(s.gx), convertRawGyro(s.gy), convertRawGyro(s.gz), convertRawAcceleration(s.ax), convertRawAcceleration(s.ay), convertRawAcceleration(s.az));
    updateControl(dt_s);
    
  }
  
    

  if (LEDF) {
    LEDF=false;
    ledTick();
  }

  if (OUTF) {
    OUTF=false;
    writeServos();
  }

  if (RXDF) {
    RXDF=false;
    if (sbus_rx.Read()) {
      if (USB) {
        ledBlink(kColorData, 10, 32);
      }else{
        ledBlink(kColorData, 90, 32);
      }
    
    /* Grab the received data */
    data = sbus_rx.data();
    //post 16 channels
      if (data.ch[6] > 1500) {
        pushCounter++;
        if (pushCounter > 64) {
          //calibrate IMU
          pushCounter=0;
          ledcOn(kColorWarning, 50);
          imu_calibrate();
          settleFilter(200, 1);
          zeroAttitude();
          
        }
      }else{
        pushCounter=0;
      }
    }
    
  }
  if (TXDF && USB) {
    TXDF=false;
    
    Serial.print("Time:");
    Serial.println(dt_s * 1000.0f);
    /*
    for (int i = 0; i < 8; i++) {
      Serial.print(data.ch[i]);
      Serial.print(",");
    }
    */
    //Serial.println(String("Roll:") + String(filter.getRoll() - rollOffset));
    //Serial.println(String("Pitch:") + String(filter.getPitch() - pitchOffset));
    //Serial.println(String("Yaw:") + String(filter.getYaw() - yawOffset));
    Serial.println(String("DesiredRollDeg:") + String(desiredRollDeg));
    Serial.println(String("DesiredPitchDeg:") + String(desiredPitchDeg));
    Serial.println(String("RollCmd:") + String(rollCmd));
    Serial.println(String("PitchCmd:") + String(pitchCmd));
    pollSerialConfig();
  }
  
}
