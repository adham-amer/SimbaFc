#include "Simba.h"


// Receiver
bfs::SbusRx sbus_rx(&Serial1, kSbusRxPin, kSbusTxPin, true, false);
bfs::SbusData data;



// Built-in LED
Adafruit_NeoPixel led(kNumLeds, kLedPin, NEO_GRB + NEO_KHZ800);


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
  

  if ((Ticks % kLedTickDivider) == 0) { // slow LED update
    LEDF=true;
  }


}

void setupTimer() {
  timer0 = timerBegin(kTimerHz); // Timer freq
  timerAttachInterrupt(timer0, &timerCall);
  timerAlarm(timer0, kTimerAlarmTicks, true, 0); // interrupts every 1000 ticks
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
