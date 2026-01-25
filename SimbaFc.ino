#include "Simba.h"
#include "mixer.h"
#include "web_page.h"
#include <WiFi.h>
#include <WebServer.h>

namespace {
const char kApSsid[] = "SimbaFc";
const char kApPass[] = "simbaconfig";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", kIndexHtml);
}

void handleData() {
  float rollDeg = filter.getRoll() - rollOffset;
  float pitchDeg = filter.getPitch() - pitchOffset;
  float yawDeg = filter.getYaw() - yawOffset;

  String json = "{";
  json += "\"roll\":" + String(rollDeg, 2) + ",";
  json += "\"pitch\":" + String(pitchDeg, 2) + ",";
  json += "\"yaw\":" + String(yawDeg, 2) + ",";
  json += "\"desiredRoll\":" + String(desiredRollDeg, 2) + ",";
  json += "\"desiredPitch\":" + String(desiredPitchDeg, 2) + ",";
  json += "\"rollCmd\":" + String(rollCmd, 2) + ",";
  json += "\"pitchCmd\":" + String(pitchCmd, 2) + ",";
  json += "\"ch\":[";
  for (int i = 0; i < 16; i++) {
    json += String(data.ch[i]);
    if (i < 15) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleGetConfig() {
  String json = "{";
  json += "\"rollKp\":" + String(gConfig.rollKp, 3) + ",";
  json += "\"rollKi\":" + String(gConfig.rollKi, 3) + ",";
  json += "\"rollKd\":" + String(gConfig.rollKd, 3) + ",";
  json += "\"pitchKp\":" + String(gConfig.pitchKp, 3) + ",";
  json += "\"pitchKi\":" + String(gConfig.pitchKi, 3) + ",";
  json += "\"pitchKd\":" + String(gConfig.pitchKd, 3) + ",";
  json += "\"maxAttitudeDeg\":" + String(gConfig.maxAttitudeDeg, 2) + ",";
  json += "\"mode1RateDegPerSec\":" + String(gConfig.mode1RateDegPerSec, 2) + ",";
  json += "\"mode3StickThreshold\":" + String(gConfig.mode3StickThreshold, 2) + ",";
  json += "\"pidOutputMin\":" + String(gConfig.pidOutputMin, 2) + ",";
  json += "\"pidOutputMax\":" + String(gConfig.pidOutputMax, 2);
  json += "}";
  server.send(200, "application/json", json);
}

bool updateFloatArg(const char* name, float* value) {
  if (!server.hasArg(name)) return false;
  *value = server.arg(name).toFloat();
  return true;
}

void handlePostConfig() {
  bool changed = false;
  changed |= updateFloatArg("rollKp", &gConfig.rollKp);
  changed |= updateFloatArg("rollKi", &gConfig.rollKi);
  changed |= updateFloatArg("rollKd", &gConfig.rollKd);
  changed |= updateFloatArg("pitchKp", &gConfig.pitchKp);
  changed |= updateFloatArg("pitchKi", &gConfig.pitchKi);
  changed |= updateFloatArg("pitchKd", &gConfig.pitchKd);
  changed |= updateFloatArg("maxAttitudeDeg", &gConfig.maxAttitudeDeg);
  changed |= updateFloatArg("mode1RateDegPerSec", &gConfig.mode1RateDegPerSec);
  changed |= updateFloatArg("mode3StickThreshold", &gConfig.mode3StickThreshold);
  changed |= updateFloatArg("pidOutputMin", &gConfig.pidOutputMin);
  changed |= updateFloatArg("pidOutputMax", &gConfig.pidOutputMax);

  gConfig.maxAttitudeDeg = constrain(gConfig.maxAttitudeDeg, 1.0f, 90.0f);
  gConfig.mode1RateDegPerSec = constrain(gConfig.mode1RateDegPerSec, 1.0f, 360.0f);
  gConfig.mode3StickThreshold = constrain(gConfig.mode3StickThreshold, 0.0f, 1.0f);
  if (gConfig.pidOutputMin > gConfig.pidOutputMax) {
    float tmp = gConfig.pidOutputMin;
    gConfig.pidOutputMin = gConfig.pidOutputMax;
    gConfig.pidOutputMax = tmp;
  }

  if (changed) {
    applyConfig();
    saveConfig();
  }
  handleGetConfig();
}

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPass);
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/config", HTTP_GET, handleGetConfig);
  server.on("/config", HTTP_POST, handlePostConfig);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
}
}

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
  loadConfig();
  applyConfig();
  startAccessPoint();
  setupWebServer();
  
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
  }
  server.handleClient();
}
