#include "Simba.h"


// Receiver
bfs::SbusRx sbus_rx(&Serial1, kSbusRxPin, kSbusTxPin, true, false);
bfs::SbusData data;



// Built-in LED
Adafruit_NeoPixel led(kNumLeds, kLedPin, NEO_GRB + NEO_KHZ800);
Color color=kColorCritical;


//filter
Madgwick filter;




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


bool begun =false;
bool builtInLedOn = false;
void builtInLed(uint8_t brightness) {
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
  	led.setPixelColor(0, led.Color(color.r, color.g, color.b)); 
  	led.show();
}
