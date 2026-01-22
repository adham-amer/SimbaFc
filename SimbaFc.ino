#include "Simba.h"



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

  ledcOn(kColorInitializing, 50);
  imu_init();


  ledcOn(kColorWarning, 50);
  imu_calibrate();
  settleFilter(200, 1);
  zeroAttitude();
  
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
    
  }
  
    

  if (LEDF) {
    LEDF=false;
    ledTick();
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
    Serial.print(dt_s * 1000.0f);
    Serial.print(",");
    
    for (int i = 0; i < 8; i++) {
      Serial.print(data.ch[i]);
      Serial.print(",");
    }
    
    Serial.print(filter.getRoll() - rollOffset);
    Serial.print(",");
    Serial.print(filter.getPitch() - pitchOffset);
    Serial.print(",");
    Serial.println(filter.getYaw() - yawOffset);
  }
  
}



