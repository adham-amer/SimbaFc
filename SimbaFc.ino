#include "Simba.h"



int lastTime = 0;


void setup() {
  Serial.begin(115200);
  delay(100);

  //InitIMU();

  sbus_rx.Begin();

  setupTimer();

  imu_init();



Serial.print(rd8(0x03));
  lastTime = micros();
}

float dt=0;
ImuSample s;
 
void loop() {

  if (PIDF) { //1khz loop
   PIDF = false;
    //calc time and dt
    unsigned long now = micros();
    dt = (now - lastTime); // * 1e-6;
    lastTime = now;
    imu_read(s);
    
  }
  
    

  if (LEDF and 1) {
    LEDF=false;
    builtInLed(200,200,0,(!builtInLedOn) *50);
    //Serial.print("Time:");
    Serial.print(dt);
    Serial.print(",");
    Serial.print(s.gx);

    Serial.print(",");
    Serial.print(s.gy);

    Serial.print(",");
    Serial.print(s.gz);

    Serial.print(",");
    Serial.print(s.ax);

    Serial.print(",");
    Serial.print(s.ay);

    Serial.print(",");
    Serial.println(s.az);
  }
  
}




