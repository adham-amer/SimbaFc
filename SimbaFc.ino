#include "Simba.h"
#include "imu.h"


uint32_t lastTimeUs = 0;



void setup() {
  Serial.begin(115200);
  delay(100);

  //InitIMU();

  sbus_rx.Begin();

  setupTimer();

  imu_init();



//Serial.print(rd8(0x03));
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
    
  }
  
    

  if (LEDF) {
    LEDF=false;
    builtInLed(200,200,0,(!builtInLedOn) *50);
    Serial.print("Time:");
    Serial.print(dt_s * 1000.0f);
    Serial.print(",");
    Serial.print(filter.getRoll());
    Serial.print(",");
    Serial.print(filter.getPitch());
    Serial.print(",");
    Serial.println(filter.getYaw());
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
