#include "Simba.h"


//Reciver
bfs::SbusRx sbus_rx(&Serial1,SBUS_RX_PIN,SBUS_TX_PIN,true,false);
bfs::SbusData data;



//BUILT-IN LED
Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


//IMU
//const int MPU = 0x68; // MPU6050 I2C address


  /*
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);
  
    
  // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000deg/s full scale)
  Wire.endTransmission(true);
  delay(20);
  */








//filter
Madgwick filter;




//timer inturupts
volatile bool PIDF=false;
volatile bool LEDF=false;
volatile int Ticks = 0;
hw_timer_t* timer0 = NULL;

void IRAM_ATTR timerCall() {
  PIDF=true;
  Ticks++;
  

  if ((Ticks % 10) == 0) {
    LEDF=true;
  }


}

void setupTimer() {
  timer0 = timerBegin(1000000); // Timer freq
  timerAttachInterrupt(timer0, &timerCall);
  timerAlarm(timer0,1000,true,0); //Inturrupts every 1000 ticks
  timerStart(timer0);
}


bool begun =false;
bool builtInLedOn = false;
void builtInLed(uint8_t red,uint8_t green,uint8_t blue,uint8_t brightness) {
	if (!begun) {
		led.begin();
    begun=true;
	}
  if (brightness==0) {
    builtInLedOn=false;
  } else {
    builtInLedOn=true;
  }
  	led.setBrightness(brightness);
  	led.setPixelColor(0, led.Color(red, green, blue)); 
  	led.show();
}
